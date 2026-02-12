/*
 * Interop test: Commands (Group 12 CROB, Group 41 Analog Output).
 *
 * Verifies that opendnp3 master can send control commands to stepfunc
 * outstation and that the outstation correctly receives them.
 */

#include "InteropFixture.h"

#include <opendnp3/app/AnalogOutput.h>
#include <opendnp3/app/ControlRelayOutputBlock.h>
#include <opendnp3/gen/CommandPointState.h>
#include <opendnp3/gen/CommandStatus.h>
#include <opendnp3/gen/OperationType.h>
#include <opendnp3/gen/TaskCompletion.h>

#include <catch.hpp>

#define SUITE(name) "InteropCommands - " name

// ---------------------------------------------------------------------------
// Helper: set up a stepfunc outstation with a CollectingControlHandler and
// an opendnp3 master connected to it. Returns the master, soeHandler, and
// control handler pointer.
// ---------------------------------------------------------------------------

struct CommandTestEnv
{
    dnp3::Runtime sfRuntime;
    dnp3::OutstationServer sfServer;
    CollectingControlHandler* ctrlHandler;
    dnp3::Outstation sfOutstation;

    opendnp3::DNP3Manager manager;
    std::shared_ptr<CollectingSOEHandler> soeHandler;
    std::shared_ptr<opendnp3::IChannel> channel;
    std::shared_ptr<opendnp3::IMaster> master;

    CommandTestEnv(uint16_t port)
        : sfRuntime(dnp3::RuntimeConfig()),
          sfServer(
              dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, MakeEndpoint(port))),
          ctrlHandler(nullptr),
          sfOutstation(SetupOutstation()),
          manager(1, opendnp3::ConsoleLogger::Create()),
          soeHandler(std::make_shared<CollectingSOEHandler>()),
          channel(nullptr),
          master(nullptr)
    {
        // Seed a binary input so the integrity poll has data to confirm connectivity
        {
            auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
                db.add_binary_input(0, dnp3::EventClass::class1, dnp3::BinaryInputConfig());
                db.update_binary_input(
                    dnp3::BinaryInput(0, true, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                    dnp3::UpdateOptions::no_event());
            });
            sfOutstation.transaction(txn);
        }

        sfServer.bind();

        channel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                       {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

        opendnp3::MasterStackConfig masterCfg;
        masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
        masterCfg.master.disableUnsolOnStartup = true;
        masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::AllClasses();
        masterCfg.link.LocalAddr = 1;
        masterCfg.link.RemoteAddr = 1024;

        master = channel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

        master->Enable();
    }

private:
    dnp3::Outstation SetupOutstation()
    {
        dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(10, 10, 10, 10, 10, 10, 10, 10));
        sfOutCfg.features.unsolicited = false;

        auto handler = std::make_unique<CollectingControlHandler>();
        ctrlHandler = handler.get();

        auto sfFilter = dnp3::AddressFilter::any();
        return sfServer.add_outstation(sfOutCfg, std::make_unique<NullOutstationApplication>(),
                                       std::make_unique<NullOutstationInformation>(), std::move(handler),
                                       std::make_unique<NullConnectionStateListener>(), sfFilter);
    }
};

// -----------------------------------------------------------------------
// Group 12: CROB SelectAndOperate
// -----------------------------------------------------------------------
TEST_CASE(SUITE("Group12_SelectAndOperate"))
{
    const uint16_t port = FindEphemeralPort();
    CommandTestEnv env(port);

    REQUIRE(env.soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    SyncCommandCallback cb;
    opendnp3::ControlRelayOutputBlock crob(opendnp3::OperationType::LATCH_ON, opendnp3::TripCloseCode::NUL, false,
                                           1,    // count
                                           1000, // on time
                                           500   // off time
    );

    env.master->SelectAndOperate(crob, 0, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.summary == opendnp3::TaskCompletion::SUCCESS);

    // Verify the outstation received the operate
    REQUIRE(env.ctrlHandler->WaitForOperates(1, INTEROP_TIMEOUT));

    {
        std::lock_guard<std::mutex> lock(env.ctrlHandler->mutex);
        REQUIRE(env.ctrlHandler->crobs.size() == 1);
        CHECK(env.ctrlHandler->crobs[0].index == 0);
        CHECK(env.ctrlHandler->crobs[0].op_type == dnp3::OperateType::select_before_operate);
        CHECK(env.ctrlHandler->crobs[0].count == 1);
        CHECK(env.ctrlHandler->crobs[0].on_time == 1000);
        CHECK(env.ctrlHandler->crobs[0].off_time == 500);
    }

    // Verify the command result
    REQUIRE(cb.results.size() == 1);
    CHECK(cb.results[0].status == opendnp3::CommandStatus::SUCCESS);
    CHECK(cb.results[0].state == opendnp3::CommandPointState::SUCCESS);

    env.master->Disable();
}

// -----------------------------------------------------------------------
// Group 12: CROB DirectOperate
// -----------------------------------------------------------------------
TEST_CASE(SUITE("Group12_DirectOperate"))
{
    const uint16_t port = FindEphemeralPort();
    CommandTestEnv env(port);

    REQUIRE(env.soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    SyncCommandCallback cb;
    opendnp3::ControlRelayOutputBlock crob(opendnp3::OperationType::PULSE_ON, opendnp3::TripCloseCode::NUL, false,
                                           2,   // count
                                           500, // on time
                                           250  // off time
    );

    env.master->DirectOperate(crob, 3, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.summary == opendnp3::TaskCompletion::SUCCESS);

    REQUIRE(env.ctrlHandler->WaitForOperates(1, INTEROP_TIMEOUT));

    {
        std::lock_guard<std::mutex> lock(env.ctrlHandler->mutex);
        REQUIRE(env.ctrlHandler->crobs.size() == 1);
        CHECK(env.ctrlHandler->crobs[0].index == 3);
        CHECK(env.ctrlHandler->crobs[0].op_type == dnp3::OperateType::direct_operate);
        CHECK(env.ctrlHandler->crobs[0].count == 2);
        CHECK(env.ctrlHandler->crobs[0].on_time == 500);
        CHECK(env.ctrlHandler->crobs[0].off_time == 250);
    }

    REQUIRE(cb.results.size() == 1);
    CHECK(cb.results[0].status == opendnp3::CommandStatus::SUCCESS);

    env.master->Disable();
}

// -----------------------------------------------------------------------
// Group 41 Var 1: AnalogOutputInt32 DirectOperate
// -----------------------------------------------------------------------
TEST_CASE(SUITE("Group41Var1_AnalogOutputInt32"))
{
    const uint16_t port = FindEphemeralPort();
    CommandTestEnv env(port);

    REQUIRE(env.soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    SyncCommandCallback cb;
    opendnp3::AnalogOutputInt32 ao(12345);
    env.master->DirectOperate(ao, 0, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.summary == opendnp3::TaskCompletion::SUCCESS);

    REQUIRE(env.ctrlHandler->WaitForOperates(1, INTEROP_TIMEOUT));

    {
        std::lock_guard<std::mutex> lock(env.ctrlHandler->mutex);
        REQUIRE(env.ctrlHandler->analogs.size() == 1);
        CHECK(env.ctrlHandler->analogs[0].index == 0);
        CHECK(env.ctrlHandler->analogs[0].value == 12345.0);
        CHECK(env.ctrlHandler->analogs[0].variation == 1);
        CHECK(env.ctrlHandler->analogs[0].op_type == dnp3::OperateType::direct_operate);
    }

    REQUIRE(cb.results.size() == 1);
    CHECK(cb.results[0].status == opendnp3::CommandStatus::SUCCESS);

    env.master->Disable();
}

// -----------------------------------------------------------------------
// Group 41 Var 2: AnalogOutputInt16 DirectOperate
// -----------------------------------------------------------------------
TEST_CASE(SUITE("Group41Var2_AnalogOutputInt16"))
{
    const uint16_t port = FindEphemeralPort();
    CommandTestEnv env(port);

    REQUIRE(env.soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    SyncCommandCallback cb;
    opendnp3::AnalogOutputInt16 ao(-4567);
    env.master->DirectOperate(ao, 1, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.summary == opendnp3::TaskCompletion::SUCCESS);

    REQUIRE(env.ctrlHandler->WaitForOperates(1, INTEROP_TIMEOUT));

    {
        std::lock_guard<std::mutex> lock(env.ctrlHandler->mutex);
        REQUIRE(env.ctrlHandler->analogs.size() == 1);
        CHECK(env.ctrlHandler->analogs[0].index == 1);
        CHECK(env.ctrlHandler->analogs[0].value == -4567.0);
        CHECK(env.ctrlHandler->analogs[0].variation == 2);
        CHECK(env.ctrlHandler->analogs[0].op_type == dnp3::OperateType::direct_operate);
    }

    REQUIRE(cb.results.size() == 1);
    CHECK(cb.results[0].status == opendnp3::CommandStatus::SUCCESS);

    env.master->Disable();
}

// -----------------------------------------------------------------------
// Group 41 Var 3: AnalogOutputFloat32 DirectOperate
// -----------------------------------------------------------------------
TEST_CASE(SUITE("Group41Var3_AnalogOutputFloat32"))
{
    const uint16_t port = FindEphemeralPort();
    CommandTestEnv env(port);

    REQUIRE(env.soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    SyncCommandCallback cb;
    opendnp3::AnalogOutputFloat32 ao(3.14f);
    env.master->DirectOperate(ao, 2, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.summary == opendnp3::TaskCompletion::SUCCESS);

    REQUIRE(env.ctrlHandler->WaitForOperates(1, INTEROP_TIMEOUT));

    {
        std::lock_guard<std::mutex> lock(env.ctrlHandler->mutex);
        REQUIRE(env.ctrlHandler->analogs.size() == 1);
        CHECK(env.ctrlHandler->analogs[0].index == 2);
        CHECK(env.ctrlHandler->analogs[0].value == Approx(3.14).epsilon(0.01));
        CHECK(env.ctrlHandler->analogs[0].variation == 3);
        CHECK(env.ctrlHandler->analogs[0].op_type == dnp3::OperateType::direct_operate);
    }

    REQUIRE(cb.results.size() == 1);
    CHECK(cb.results[0].status == opendnp3::CommandStatus::SUCCESS);

    env.master->Disable();
}

// -----------------------------------------------------------------------
// Group 41 Var 4: AnalogOutputDouble64 DirectOperate
// -----------------------------------------------------------------------
TEST_CASE(SUITE("Group41Var4_AnalogOutputDouble64"))
{
    const uint16_t port = FindEphemeralPort();
    CommandTestEnv env(port);

    REQUIRE(env.soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    SyncCommandCallback cb;
    opendnp3::AnalogOutputDouble64 ao(2.71828);
    env.master->DirectOperate(ao, 5, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.summary == opendnp3::TaskCompletion::SUCCESS);

    REQUIRE(env.ctrlHandler->WaitForOperates(1, INTEROP_TIMEOUT));

    {
        std::lock_guard<std::mutex> lock(env.ctrlHandler->mutex);
        REQUIRE(env.ctrlHandler->analogs.size() == 1);
        CHECK(env.ctrlHandler->analogs[0].index == 5);
        CHECK(env.ctrlHandler->analogs[0].value == Approx(2.71828).epsilon(0.0001));
        CHECK(env.ctrlHandler->analogs[0].variation == 4);
        CHECK(env.ctrlHandler->analogs[0].op_type == dnp3::OperateType::direct_operate);
    }

    REQUIRE(cb.results.size() == 1);
    CHECK(cb.results[0].status == opendnp3::CommandStatus::SUCCESS);

    env.master->Disable();
}
