/*
 * Interop test: Binary Output Status (Group 10/11).
 *
 * Verifies that binary output status data is correctly exchanged between
 * opendnp3 and stepfunc/dnp3 in both directions.
 */

#include "InteropFixture.h"

#include <catch.hpp>

#define SUITE(name) "InteropBinaryOutputStatus - " name

// -----------------------------------------------------------------------
// Test: opendnp3 master reads binary output status from stepfunc outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("OurMasterReadsBOStatusFromTheirOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with binary output status -------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 10, 0, 0, 0, 0, 0));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed binary output status data
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            db.add_binary_output_status(0, dnp3::EventClass::class1, dnp3::BinaryOutputStatusConfig());
            db.update_binary_output_status(
                dnp3::BinaryOutputStatus(0, true, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            db.add_binary_output_status(1, dnp3::EventClass::class1, dnp3::BinaryOutputStatusConfig());
            db.update_binary_output_status(
                dnp3::BinaryOutputStatus(1, false, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
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

        REQUIRE(soeHandler->binary_output_statuses.size() >= 2);
        CHECK(soeHandler->binary_output_statuses[0].index == 0);
        CHECK(soeHandler->binary_output_statuses[0].value == true);
        CHECK(soeHandler->binary_output_statuses[1].index == 1);
        CHECK(soeHandler->binary_output_statuses[1].value == false);
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: stepfunc master reads binary output status from opendnp3 outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("TheirMasterReadsBOStatusFromOurOutstation"))
{
    const uint16_t port = FindEphemeralPort();

    // -- opendnp3 outstation with binary output status ------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::DatabaseConfig dbCfg;
    dbCfg.binary_output_status[0] = opendnp3::BOStatusConfig();
    dbCfg.binary_output_status[1] = opendnp3::BOStatusConfig();

    opendnp3::OutstationStackConfig outstationCfg(dbCfg);
    outstationCfg.outstation.params.allowUnsolicited = false;
    outstationCfg.link.LocalAddr = 1024;
    outstationCfg.link.RemoteAddr = 1;

    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outstationCfg);

    // Seed binary output status
    opendnp3::UpdateBuilder builder;
    builder.Update(opendnp3::BinaryOutputStatus(true), 0);
    builder.Update(opendnp3::BinaryOutputStatus(false), 1);
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

        REQUIRE(handlerPtr->binary_output_statuses.size() >= 2);
        CHECK(handlerPtr->binary_output_statuses[0].index == 0);
        CHECK(handlerPtr->binary_output_statuses[0].value == true);
        CHECK(handlerPtr->binary_output_statuses[1].index == 1);
        CHECK(handlerPtr->binary_output_statuses[1].value == false);
    }
}
