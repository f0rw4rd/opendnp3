/*
 * Interop test: Write Dead Bands (Group 34) and Check Link Status.
 *
 * Tests opendnp3 master writing analog input dead-bands to a stepfunc
 * outstation, and manually checking link status on a live connection.
 */

#include "InteropFixture.h"

#include <catch.hpp>

#define SUITE(name) "InteropWriteDeadBands - " name

// ---------------------------------------------------------------------------
// Synchronizing callback for opendnp3 FileOperationResult
// ---------------------------------------------------------------------------
class SyncOpCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::FileOperationResult result;

    opendnp3::FileOperationCallbackT Callback()
    {
        return [this](const opendnp3::FileOperationResult& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// Synchronizing callback for CheckLinkStatus (bool result)
// ---------------------------------------------------------------------------
class SyncLinkStatusCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    bool success = false;

    std::function<void(bool)> Callback()
    {
        return [this](bool result) {
            std::lock_guard<std::mutex> lock(mutex);
            success = result;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// stepfunc outstation application that supports writing dead bands
// ---------------------------------------------------------------------------
class DeadBandOutstationApplication : public dnp3::OutstationApplication
{
public:
    std::mutex mutex;
    std::vector<std::pair<uint16_t, double>> written_dead_bands;

    uint16_t get_processing_delay_ms() override
    {
        return 0;
    }

    dnp3::WriteTimeResult write_absolute_time(uint64_t /*time*/) override
    {
        return dnp3::WriteTimeResult::not_supported;
    }

    dnp3::ApplicationIin get_application_iin() override
    {
        return dnp3::ApplicationIin();
    }

    dnp3::RestartDelay cold_restart() override
    {
        return dnp3::RestartDelay::not_supported();
    }

    dnp3::RestartDelay warm_restart() override
    {
        return dnp3::RestartDelay::not_supported();
    }

    bool support_write_analog_dead_bands() override
    {
        return true;
    }

    void begin_write_analog_dead_bands() override {}

    void write_analog_dead_band(uint16_t index, double dead_band) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        written_dead_bands.push_back({index, dead_band});
    }

    void end_write_analog_dead_bands() override {}
};

// ---------------------------------------------------------------------------
// Test: opendnp3 master writes dead bands to stepfunc outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("WriteDeadBandsToStepfuncOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation that supports dead band writes -------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig::no_events());
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfApp = std::make_shared<DeadBandOutstationApplication>();
    auto* sfAppPtr = sfApp.get();

    // We need a custom shared_ptr wrapper that copies into unique_ptr for stepfunc
    auto sfAppUnique = std::unique_ptr<DeadBandOutstationApplication>(new DeadBandOutstationApplication());
    auto* sfAppRawPtr = sfAppUnique.get();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::move(sfAppUnique), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Add analog inputs so the outstation has points to associate dead bands with
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            dnp3::AnalogInputConfig analogCfg(dnp3::StaticAnalogInputVariation::group30_var5,
                                              dnp3::EventAnalogInputVariation::group32_var5, 0.0);
            db.add_analog_input(0, dnp3::EventClass::class1, analogCfg);
            db.update_analog_input(
                dnp3::AnalogInput(0, 10.0, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            db.add_analog_input(1, dnp3::EventClass::class1, analogCfg);
            db.update_analog_input(
                dnp3::AnalogInput(1, 20.0, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            db.add_analog_input(2, dnp3::EventClass::class1, analogCfg);
            db.update_analog_input(
                dnp3::AnalogInput(2, 30.0, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    // -- opendnp3 master writes dead bands -----------------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    auto channel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                        {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::None();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = channel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();

    // Wait for connection to establish
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Build dead band write request
    std::vector<opendnp3::Indexed<opendnp3::AnalogInputDeadband>> deadBands;
    deadBands.push_back(opendnp3::WithIndex(opendnp3::AnalogInputDeadband(1.5), 0));
    deadBands.push_back(opendnp3::WithIndex(opendnp3::AnalogInputDeadband(2.5), 1));
    deadBands.push_back(opendnp3::WithIndex(opendnp3::AnalogInputDeadband(3.5), 2));

    SyncOpCallback cb;
    master->WriteDeadBands(deadBands, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);

    // Verify the stepfunc outstation received the dead band values
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(sfAppRawPtr->mutex);
        REQUIRE(sfAppRawPtr->written_dead_bands.size() == 3);
        CHECK(sfAppRawPtr->written_dead_bands[0].first == 0);
        CHECK(sfAppRawPtr->written_dead_bands[0].second == Approx(1.5).margin(0.01));
        CHECK(sfAppRawPtr->written_dead_bands[1].first == 1);
        CHECK(sfAppRawPtr->written_dead_bands[1].second == Approx(2.5).margin(0.01));
        CHECK(sfAppRawPtr->written_dead_bands[2].first == 2);
        CHECK(sfAppRawPtr->written_dead_bands[2].second == Approx(3.5).margin(0.01));
    }

    master->Disable();
}

// ---------------------------------------------------------------------------
// Test: opendnp3 master checks link status against stepfunc outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("CheckLinkStatusAgainstStepfuncOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation --------------------------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig::no_events());
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    sfServer.bind();

    // -- opendnp3 master ------------------------------------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    auto channel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                        {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::None();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = channel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();

    // Wait for connection to establish
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Check link status - should succeed since we're connected
    SyncLinkStatusCallback cb;
    master->CheckLinkStatus(cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.success == true);

    master->Disable();
}
