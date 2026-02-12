/*
 * Interop test: Octet String (Group 110/111).
 *
 * Verifies that octet string data is correctly exchanged between
 * opendnp3 and stepfunc/dnp3 in both directions.
 */

#include "InteropFixture.h"

#include <catch.hpp>

#define SUITE(name) "InteropOctetString - " name

// -----------------------------------------------------------------------
// Test: opendnp3 master reads octet string from stepfunc outstation
// via explicit ScanRange
// -----------------------------------------------------------------------
TEST_CASE(SUITE("OurMasterReadsOctetStringFromTheirOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with octet string data ---------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 0, 0, 0, 0, 0, 10));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed octet string data and a binary input to confirm connectivity
    const std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    {
        auto txn = dnp3::functional::database_transaction([&testData](dnp3::Database& db) {
            db.add_binary_input(0, dnp3::EventClass::class1, dnp3::BinaryInputConfig());
            db.update_binary_input(
                dnp3::BinaryInput(0, true, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            db.add_octet_string(0, dnp3::EventClass::class1);
            db.update_octet_string(0, testData, dnp3::UpdateOptions::no_event());
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

    // Wait for connection via the integrity poll (binary input arrives)
    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    // Explicitly scan for Group 110 Var 0 (octet string) at index 0
    master->ScanRange(opendnp3::GroupVariationID(110, 0), 0, 0, soeHandler);

    // Wait for the scan response
    REQUIRE(soeHandler->WaitForFragments(2, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        REQUIRE(soeHandler->octet_strings.size() >= 1);
        CHECK(soeHandler->octet_strings[0].index == 0);
        CHECK(soeHandler->octet_strings[0].data == testData);
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: opendnp3 master receives octet string during integrity poll
// when stepfunc outstation includes octet strings in Class 0
// -----------------------------------------------------------------------
TEST_CASE(SUITE("OurMasterReadsOctetStringViaIntegrityPoll"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with octet string in Class 0 ---------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(0, 0, 0, 0, 0, 0, 0, 10));
    sfOutCfg.features.unsolicited = false;
    // Enable octet strings in Class 0 so they appear in integrity poll responses
    sfOutCfg.class_zero.octet_string = true;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    const std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    {
        auto txn = dnp3::functional::database_transaction([&testData](dnp3::Database& db) {
            db.add_octet_string(0, dnp3::EventClass::class1);
            db.update_octet_string(0, testData, dnp3::UpdateOptions::no_event());
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    // -- opendnp3 master with integrity poll ----------------------------------
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

    // Wait for the integrity poll to complete
    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        // Octet string should have arrived during the integrity poll
        REQUIRE(soeHandler->octet_strings.size() >= 1);
        CHECK(soeHandler->octet_strings[0].index == 0);
        CHECK(soeHandler->octet_strings[0].data == testData);
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: stepfunc master reads octet string from opendnp3 outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("TheirMasterReadsOctetStringFromOurOutstation"))
{
    const uint16_t port = FindEphemeralPort();

    // -- opendnp3 outstation with octet string data ---------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::DatabaseConfig dbCfg;
    dbCfg.octet_string[0] = opendnp3::OctetStringConfig();

    opendnp3::OutstationStackConfig outstationCfg(dbCfg);
    outstationCfg.outstation.params.allowUnsolicited = false;
    outstationCfg.link.LocalAddr = 1024;
    outstationCfg.link.RemoteAddr = 1;

    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outstationCfg);

    // Seed octet string data -- opendnp3 OctetString takes a Buffer
    const uint8_t rawData[] = {0x57, 0x6F, 0x72, 0x6C, 0x64}; // "World"
    opendnp3::OctetString os(opendnp3::Buffer(rawData, sizeof(rawData)));

    opendnp3::UpdateBuilder builder;
    builder.Update(os, 0);
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

        REQUIRE(handlerPtr->octet_strings.size() >= 1);
        CHECK(handlerPtr->octet_strings[0].index == 0);

        const std::vector<uint8_t> expected = {0x57, 0x6F, 0x72, 0x6C, 0x64};
        CHECK(handlerPtr->octet_strings[0].data == expected);
    }
}
