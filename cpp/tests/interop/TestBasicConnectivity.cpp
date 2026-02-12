/*
 * Interop test: basic TCP connectivity between opendnp3 and stepfunc/dnp3.
 *
 * Test 4a: Our master talks to their outstation and vice versa.
 */

#include "InteropFixture.h"

#include <catch.hpp>

#define SUITE(name) "InteropBasicConnectivity - " name

// -----------------------------------------------------------------------
// Test: opendnp3 master  <-->  stepfunc outstation
// -----------------------------------------------------------------------
TEST_CASE(SUITE("OurMasterTheirOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation --------------------------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, // outstation address
                                    1,    // master address
                                    dnp3::EventBufferConfig(10, 10, 10, 10, 10, 10, 10, 10));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed the database with some static values
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            db.add_binary_input(0, dnp3::EventClass::class1, dnp3::BinaryInputConfig());
            db.update_binary_input(
                dnp3::BinaryInput(0, true, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            dnp3::AnalogInputConfig analogCfg(dnp3::StaticAnalogInputVariation::group30_var5,
                                              dnp3::EventAnalogInputVariation::group32_var5, 0.0);
            db.add_analog_input(0, dnp3::EventClass::class1, analogCfg);
            db.update_analog_input(
                dnp3::AnalogInput(0, 42.5, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                dnp3::UpdateOptions::no_event());

            db.add_counter(0, dnp3::EventClass::class1, dnp3::CounterConfig());
            db.update_counter(dnp3::Counter(0, 100, dnp3::Flags(0x01), dnp3::Timestamp::synchronized_timestamp(0)),
                              dnp3::UpdateOptions::no_event());
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    // -- opendnp3 master ------------------------------------------------------
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

    // Wait for the integrity poll to complete and data to arrive
    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    // Give a moment for all data to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        // Verify binary input
        REQUIRE(soeHandler->binaries.size() >= 1);
        CHECK(soeHandler->binaries[0].index == 0);
        CHECK(soeHandler->binaries[0].value == true);

        // Verify analog input
        REQUIRE(soeHandler->analogs.size() >= 1);
        CHECK(soeHandler->analogs[0].index == 0);
        CHECK(soeHandler->analogs[0].value == Approx(42.5));

        // Verify counter
        REQUIRE(soeHandler->counters.size() >= 1);
        CHECK(soeHandler->counters[0].index == 0);
        CHECK(soeHandler->counters[0].value == 100);
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: stepfunc master  <-->  opendnp3 outstation
// -----------------------------------------------------------------------

TEST_CASE(SUITE("TheirMasterOurOutstation"))
{
    const uint16_t port = FindEphemeralPort();

    // -- opendnp3 outstation --------------------------------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::DatabaseConfig dbCfg;
    dbCfg.binary_input[0] = opendnp3::BinaryConfig();
    dbCfg.analog_input[0] = opendnp3::AnalogConfig();
    dbCfg.counter[0] = opendnp3::CounterConfig();

    opendnp3::OutstationStackConfig outstationCfg(dbCfg);
    outstationCfg.outstation.params.allowUnsolicited = false;
    outstationCfg.link.LocalAddr = 1024;
    outstationCfg.link.RemoteAddr = 1;

    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outstationCfg);

    // Seed the database
    opendnp3::UpdateBuilder builder;
    builder.Update(opendnp3::Binary(true), 0);
    builder.Update(opendnp3::Analog(99.0), 0);
    builder.Update(opendnp3::Counter(42), 0);
    outstation->Apply(builder.Build());

    outstation->Enable();

    // -- stepfunc master ------------------------------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    dnp3::MasterChannelConfig sfMasterCfg(1);
    dnp3::EndpointList endpoints(MakeEndpoint(port));

    auto sfMaster
        = dnp3::MasterChannel::create_tcp_channel(sfRuntime, dnp3::LinkErrorMode::close, sfMasterCfg, endpoints,
                                                  dnp3::ConnectStrategy(), std::make_unique<NullClientStateListener>());

    auto handler = std::make_unique<CollectingReadHandler>();
    auto* handlerPtr = handler.get();

    dnp3::AssociationConfig assocCfg(dnp3::EventClasses::none(), dnp3::EventClasses::none(),
                                     dnp3::Classes(true, false, false, false), // integrity poll = class 0 only
                                     dnp3::EventClasses::none());

    sfMaster.add_association(
        1024, assocCfg, std::move(handler),
        dnp3::functional::association_handler([]() -> dnp3::UtcTimestamp { return dnp3::UtcTimestamp::invalid(); }),
        std::make_unique<NullAssociationInfo>());

    sfMaster.enable();

    // Wait for the startup integrity poll to deliver data
    REQUIRE(handlerPtr->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(handlerPtr->mutex);

        // Verify binary input
        REQUIRE(handlerPtr->binaries.size() >= 1);
        CHECK(handlerPtr->binaries[0].index == 0);
        CHECK(handlerPtr->binaries[0].value == true);

        // Verify counter
        REQUIRE(handlerPtr->counters.size() >= 1);
        CHECK(handlerPtr->counters[0].index == 0);
        CHECK(handlerPtr->counters[0].value == 42);
    }
}
