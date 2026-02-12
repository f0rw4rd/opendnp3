/*
 * Interop test: Frozen Analog, Frozen Counter, and Freeze Request.
 *
 * Test 4e: stepfunc outstation reports frozen counters and frozen analog data,
 *          opendnp3 master correctly reads the values.
 *
 * Test 4f: opendnp3 master sends freeze requests to stepfunc outstation.
 */

#include "InteropFixture.h"

#include <opendnp3/master/HeaderTypes.h>

#include <catch.hpp>

#define SUITE(name) "InteropFrozenAnalogCounters - " name

// -----------------------------------------------------------------------
// Outstation application that supports freeze operations
// -----------------------------------------------------------------------
class FreezeSupportingOutstationApp : public NullOutstationApplication
{
public:
    std::atomic<int> freeze_all_count{0};
    std::atomic<int> freeze_range_count{0};

    dnp3::FreezeResult freeze_counters_all(dnp3::FreezeType /*freeze_type*/,
                                           dnp3::DatabaseHandle& database_handle) override
    {
        ++freeze_all_count;

        // Copy counter values to frozen counters
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            auto counter = db.get_counter(0);
            db.update_frozen_counter(
                dnp3::FrozenCounter(0, counter.value, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::detect_event());
        });
        database_handle.transaction(txn);

        return dnp3::FreezeResult::ok;
    }

    dnp3::FreezeResult freeze_counters_range(uint16_t /*start*/,
                                             uint16_t /*stop*/,
                                             dnp3::FreezeType /*freeze_type*/,
                                             dnp3::DatabaseHandle& database_handle) override
    {
        ++freeze_range_count;

        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            auto counter = db.get_counter(0);
            db.update_frozen_counter(
                dnp3::FrozenCounter(0, counter.value, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::detect_event());
        });
        database_handle.transaction(txn);

        return dnp3::FreezeResult::ok;
    }
};

// -----------------------------------------------------------------------
// Test 4e: Read frozen counters and frozen analog from stepfunc outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("ReadFrozenCounterFromStepfuncOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with frozen counter data -------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 0, 10, 10, 10, 0, 0));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed database with frozen counter and frozen analog values
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            // Counter
            db.add_counter(0, dnp3::EventClass::class1, dnp3::CounterConfig());
            db.update_counter(dnp3::Counter(0, 500, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                              dnp3::UpdateOptions::no_event());

            // Frozen counter
            db.add_frozen_counter(0, dnp3::EventClass::class1, dnp3::FrozenCounterConfig());
            db.update_frozen_counter(
                dnp3::FrozenCounter(0, 250, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            // Analog input (use float variation so fractional values survive)
            dnp3::AnalogInputConfig analogCfg(dnp3::StaticAnalogInputVariation::group30_var5,
                                              dnp3::EventAnalogInputVariation::group32_var5, 0.0);
            db.add_analog_input(0, dnp3::EventClass::class1, analogCfg);
            db.update_analog_input(
                dnp3::AnalogInput(0, 123.456, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    // -- opendnp3 master reads the data ---------------------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    auto channel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                        {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::AllClasses();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = channel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();

    // Wait for the integrity poll
    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        // Verify counter
        REQUIRE(soeHandler->counters.size() >= 1);
        CHECK(soeHandler->counters[0].index == 0);
        CHECK(soeHandler->counters[0].value == 500);

        // Verify analog
        REQUIRE(soeHandler->analogs.size() >= 1);
        CHECK(soeHandler->analogs[0].index == 0);
        CHECK(soeHandler->analogs[0].value == Approx(123.456).epsilon(0.001));
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: Verify frozen counters read from stepfunc outstation are captured
// -----------------------------------------------------------------------
TEST_CASE(SUITE("ReadFrozenCounterValuesFromStepfuncOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 0, 10, 0, 0, 0, 0));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            db.add_frozen_counter(0, dnp3::EventClass::class1, dnp3::FrozenCounterConfig());
            db.update_frozen_counter(
                dnp3::FrozenCounter(0, 999, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    auto channel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                        {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::AllClasses();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = channel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();

    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        REQUIRE(soeHandler->frozen_counters.size() >= 1);
        CHECK(soeHandler->frozen_counters[0].index == 0);
        CHECK(soeHandler->frozen_counters[0].value == 999);
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: stepfunc master reads frozen counter from opendnp3 outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("TheirMasterReadsFrozenCounterFromOurOutstation"))
{
    const uint16_t port = FindEphemeralPort();

    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::DatabaseConfig dbCfg;
    dbCfg.counter[0] = opendnp3::CounterConfig();
    dbCfg.frozen_counter[0] = opendnp3::FrozenCounterConfig();

    opendnp3::OutstationStackConfig outstationCfg(dbCfg);
    outstationCfg.outstation.params.allowUnsolicited = false;
    outstationCfg.link.LocalAddr = 1024;
    outstationCfg.link.RemoteAddr = 1;

    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outstationCfg);

    // Set counter to 333, then freeze it
    {
        opendnp3::UpdateBuilder builder;
        builder.Update(opendnp3::Counter(333), 0);
        outstation->Apply(builder.Build());
    }
    {
        opendnp3::UpdateBuilder builder;
        builder.FreezeCounter(0, false);
        outstation->Apply(builder.Build());
    }

    outstation->Enable();

    // -- stepfunc master reads ------------------------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    dnp3::MasterChannelConfig sfMasterCfg(1);
    dnp3::EndpointList endpoints(MakeEndpoint(port));

    auto sfMaster
        = dnp3::MasterChannel::create_tcp_channel(sfRuntime, dnp3::LinkErrorMode::close, sfMasterCfg, endpoints,
                                                  dnp3::ConnectStrategy(), std::make_unique<NullClientStateListener>());

    auto handler = std::make_unique<CollectingReadHandler>();
    auto* handlerPtr = handler.get();

    dnp3::AssociationConfig assocCfg(dnp3::EventClasses::none(), dnp3::EventClasses::none(),
                                     dnp3::Classes(true, false, false, false), dnp3::EventClasses::none());

    sfMaster.add_association(
        1024, assocCfg, std::move(handler),
        dnp3::functional::association_handler([]() -> dnp3::UtcTimestamp { return dnp3::UtcTimestamp::invalid(); }),
        std::make_unique<NullAssociationInfo>());

    sfMaster.enable();

    REQUIRE(handlerPtr->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(handlerPtr->mutex);

        REQUIRE(handlerPtr->frozen_counters.size() >= 1);
        CHECK(handlerPtr->frozen_counters[0].index == 0);
        CHECK(handlerPtr->frozen_counters[0].value == 333);
    }
}

// -----------------------------------------------------------------------
// Test 4f: opendnp3 master sends freeze request to stepfunc outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("FreezeRequestFromOurMaster"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with freeze support ------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 0, 10, 10, 0, 0, 0));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto freezeApp = std::make_unique<FreezeSupportingOutstationApp>();
    auto* freezeAppPtr = freezeApp.get();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::move(freezeApp), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed counters
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            db.add_counter(0, dnp3::EventClass::class1, dnp3::CounterConfig());
            db.update_counter(dnp3::Counter(0, 777, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                              dnp3::UpdateOptions::no_event());

            db.add_frozen_counter(0, dnp3::EventClass::class1, dnp3::FrozenCounterConfig());
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    // -- opendnp3 master sends freeze -----------------------------------------
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

    // Wait for connection
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Send an immediate freeze request using opendnp3's Freeze API.
    // FC=0x07 (IMMED_FREEZE) with an all-objects header for counters (Group 20 Var 0).
    master->Freeze(opendnp3::FreezeType::ImmediateFreeze, {opendnp3::Header::AllObjects(20, 0)});

    // Wait for the freeze to be processed
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Verify the outstation received and processed the freeze
    CHECK(freezeAppPtr->freeze_all_count.load() >= 1);

    master->Disable();
}
