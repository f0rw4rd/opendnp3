/*
 * Interop test: Analog Output Status (Group 40/42).
 *
 * Verifies that analog output status data is correctly exchanged between
 * opendnp3 and stepfunc/dnp3 in both directions.
 */

#include "InteropFixture.h"

#include <catch.hpp>

#define SUITE(name) "InteropAnalogOutputStatus - " name

// -----------------------------------------------------------------------
// Test: opendnp3 master reads analog output status from stepfunc outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("OurMasterReadsAOStatusFromTheirOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with analog output status -------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 0, 0, 0, 0, 10, 0));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed analog output status data with float variation to preserve precision
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            dnp3::AnalogOutputStatusConfig aoCfg(dnp3::StaticAnalogOutputStatusVariation::group40_var3,
                                                 dnp3::EventAnalogOutputStatusVariation::group42_var5, 0.0);

            db.add_analog_output_status(0, dnp3::EventClass::class1, aoCfg);
            db.update_analog_output_status(
                dnp3::AnalogOutputStatus(0, 75.25, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            db.add_analog_output_status(1, dnp3::EventClass::class1, aoCfg);
            db.update_analog_output_status(
                dnp3::AnalogOutputStatus(1, -10.5, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
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

    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        REQUIRE(soeHandler->analog_output_statuses.size() >= 2);
        CHECK(soeHandler->analog_output_statuses[0].index == 0);
        CHECK(soeHandler->analog_output_statuses[0].value == Approx(75.25).epsilon(0.01));
        CHECK(soeHandler->analog_output_statuses[1].index == 1);
        CHECK(soeHandler->analog_output_statuses[1].value == Approx(-10.5).epsilon(0.01));
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: stepfunc master reads analog output status from opendnp3 outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("TheirMasterReadsAOStatusFromOurOutstation"))
{
    const uint16_t port = FindEphemeralPort();

    // -- opendnp3 outstation with analog output status ------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::DatabaseConfig dbCfg;
    dbCfg.analog_output_status[0] = opendnp3::AOStatusConfig();
    dbCfg.analog_output_status[1] = opendnp3::AOStatusConfig();

    opendnp3::OutstationStackConfig outstationCfg(dbCfg);
    outstationCfg.outstation.params.allowUnsolicited = false;
    outstationCfg.link.LocalAddr = 1024;
    outstationCfg.link.RemoteAddr = 1;

    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outstationCfg);

    // Seed analog output status data
    opendnp3::UpdateBuilder builder;
    builder.Update(opendnp3::AnalogOutputStatus(100.0), 0);
    builder.Update(opendnp3::AnalogOutputStatus(-55.0), 1);
    outstation->Apply(builder.Build());

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

        REQUIRE(handlerPtr->analog_output_statuses.size() >= 2);
        CHECK(handlerPtr->analog_output_statuses[0].index == 0);
        // opendnp3 default static variation for AO status is Group40Var1 (32-bit int with flag)
        // so the 100.0 gets truncated to integer 100
        CHECK(handlerPtr->analog_output_statuses[0].value == Approx(100.0).epsilon(0.01));
        CHECK(handlerPtr->analog_output_statuses[1].index == 1);
        CHECK(handlerPtr->analog_output_statuses[1].value == Approx(-55.0).epsilon(0.01));
    }
}
