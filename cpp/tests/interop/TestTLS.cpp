/*
 * Interop test: TLS connections between opendnp3 and stepfunc/dnp3.
 *
 * Test 4d: TLS master-outstation connectivity using self-signed certificates.
 * Uses the existing test certificates from cpp/tests/asiotests/certs/.
 */

#include "InteropFixture.h"

#include <opendnp3/channel/TLSConfig.h>

#include <catch.hpp>

// The TLS tests are only compiled when TLS support is enabled
#ifdef OPENDNP3_USE_TLS

#define SUITE(name) "InteropTLS - " name

// Paths to test certificates (set by CMake via compile definition)
static const std::string SELF_SIGNED_CERT_DIR = std::string(INTEROP_CERT_DIR) + "/self_signed/";
static const std::string CA_CHAIN_CERT_DIR = std::string(INTEROP_CERT_DIR) + "/ca_chain/";

// -----------------------------------------------------------------------
// Test: opendnp3 TLS master <--> stepfunc TLS outstation (self-signed)
// -----------------------------------------------------------------------
TEST_CASE(SUITE("OurTLSMasterTheirTLSOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc TLS outstation server ---------------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    dnp3::TlsServerConfig sfTlsCfg("*",                                       // dns_name - accept any client name
                                   SELF_SIGNED_CERT_DIR + "entity1_cert.pem", // peer cert
                                   SELF_SIGNED_CERT_DIR + "entity2_cert.pem", // local cert
                                   SELF_SIGNED_CERT_DIR + "entity2_key.pem",  // private key
                                   ""                                         // no password
    );
    sfTlsCfg.certificate_mode = dnp3::CertificateMode::self_signed;
    sfTlsCfg.allow_client_name_wildcard = true;

    auto sfServer
        = dnp3::OutstationServer::create_tls_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint, sfTlsCfg);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig(5, 0, 0, 5, 0, 5, 0, 0));
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Seed data
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

    // -- opendnp3 TLS master -------------------------------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    auto channel = manager.AddTLSClient("tls-client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                        {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1",
                                        opendnp3::TLSConfig(SELF_SIGNED_CERT_DIR + "entity2_cert.pem", // peer cert
                                                            SELF_SIGNED_CERT_DIR + "entity1_cert.pem", // local cert
                                                            SELF_SIGNED_CERT_DIR + "entity1_key.pem"   // private key
                                                            ),
                                        nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::AllClasses();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = channel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();

    // Wait for TLS handshake and integrity poll
    REQUIRE(soeHandler->WaitForFragments(1, INTEROP_TIMEOUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);
        REQUIRE(soeHandler->binaries.size() >= 1);
        CHECK(soeHandler->binaries[0].value == true);
    }

    master->Disable();
}

// -----------------------------------------------------------------------
// Test: stepfunc TLS master <--> opendnp3 TLS outstation (self-signed)
// -----------------------------------------------------------------------
TEST_CASE(SUITE("TheirTLSMasterOurTLSOutstation"))
{
    const uint16_t port = FindEphemeralPort();

    // -- opendnp3 TLS outstation ----------------------------------------------
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTLSServer("tls-server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port),
                               opendnp3::TLSConfig(SELF_SIGNED_CERT_DIR + "entity1_cert.pem", // peer cert
                                                   SELF_SIGNED_CERT_DIR + "entity2_cert.pem", // local cert
                                                   SELF_SIGNED_CERT_DIR + "entity2_key.pem"   // private key
                                                   ),
                               nullptr);

    opendnp3::DatabaseConfig dbCfg;
    dbCfg.binary_input[0] = opendnp3::BinaryConfig();

    opendnp3::OutstationStackConfig outstationCfg(dbCfg);
    outstationCfg.outstation.params.allowUnsolicited = false;
    outstationCfg.link.LocalAddr = 1024;
    outstationCfg.link.RemoteAddr = 1;

    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outstationCfg);

    opendnp3::UpdateBuilder builder;
    builder.Update(opendnp3::Binary(true), 0);
    outstation->Apply(builder.Build());

    outstation->Enable();

    // -- stepfunc TLS master --------------------------------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    dnp3::MasterChannelConfig sfMasterCfg(1);
    dnp3::EndpointList endpoints(MakeEndpoint(port));

    dnp3::TlsClientConfig sfTlsCfg("*",                                       // dns_name wildcard
                                   SELF_SIGNED_CERT_DIR + "entity2_cert.pem", // peer cert
                                   SELF_SIGNED_CERT_DIR + "entity1_cert.pem", // local cert
                                   SELF_SIGNED_CERT_DIR + "entity1_key.pem",  // private key
                                   ""                                         // no password
    );
    sfTlsCfg.certificate_mode = dnp3::CertificateMode::self_signed;
    sfTlsCfg.allow_server_name_wildcard = true;

    auto sfMaster = dnp3::MasterChannel::create_tls_channel(sfRuntime, dnp3::LinkErrorMode::close, sfMasterCfg,
                                                            endpoints, dnp3::ConnectStrategy(),
                                                            std::make_unique<NullClientStateListener>(), sfTlsCfg);

    auto handler = std::make_unique<CollectingReadHandler>();
    auto* handlerPtr = handler.get();

    dnp3::AssociationConfig assocCfg(dnp3::EventClasses::none(), dnp3::EventClasses::none(),
                                     dnp3::Classes(true, false, false, false), dnp3::EventClasses::none());

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
        REQUIRE(handlerPtr->binaries.size() >= 1);
        CHECK(handlerPtr->binaries[0].value == true);
    }
}

#endif // OPENDNP3_USE_TLS
