/*
 * Interop test: Group 0 device attributes.
 *
 * Test 4b: stepfunc outstation configured with device attributes,
 * opendnp3 master reads them and verifies the parsed attribute values.
 */

#include "InteropFixture.h"

#include <catch.hpp>

#define SUITE(name) "InteropDeviceAttributes - " name

TEST_CASE(SUITE("ReadDeviceAttributesFromStepfuncOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation with device attributes ---------------------------
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig::no_events());
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    auto sfOutstation = sfServer.add_outstation(
        sfOutCfg, std::make_unique<NullOutstationApplication>(), std::make_unique<NullOutstationInformation>(),
        std::make_unique<NullControlHandler>(), std::make_unique<NullConnectionStateListener>(), sfFilter);

    // Define device attributes
    {
        auto txn = dnp3::functional::database_transaction([](dnp3::Database& db) {
            // Variation 252 = Device manufacturer's name
            db.define_string_attr(0, false, dnp3::attribute_variations::device_manufacturers_name, "TestManufacturer");

            // Variation 250 = Product name and model
            db.define_string_attr(0, false, dnp3::attribute_variations::product_name_and_model, "InteropTestDevice");

            // Variation 248 = Device serial number
            db.define_string_attr(0, false, dnp3::attribute_variations::device_serial_number, "SN-12345");
        });
        sfOutstation.transaction(txn);
    }

    sfServer.bind();

    // -- opendnp3 master reads device attributes ------------------------------
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

    // Scan for device attribute variation 252 (manufacturer name)
    master->ScanRange(opendnp3::GroupVariationID(0, 252), 0, 0, soeHandler);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Scan for variation 250 (product name)
    master->ScanRange(opendnp3::GroupVariationID(0, 250), 0, 0, soeHandler);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Scan for variation 248 (serial number)
    master->ScanRange(opendnp3::GroupVariationID(0, 248), 0, 0, soeHandler);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Verify we received the device attribute callbacks
    {
        std::lock_guard<std::mutex> lock(soeHandler->mutex);

        // We should have received at least 3 device attribute values
        REQUIRE(soeHandler->device_attributes.size() >= 3);

        // Check manufacturer name (variation 252)
        bool foundManufacturer = false;
        bool foundProduct = false;
        bool foundSerial = false;

        for (const auto& attr : soeHandler->device_attributes)
        {
            if (attr.variation == 252)
            {
                foundManufacturer = true;
                CHECK(attr.value.type == opendnp3::DeviceAttrType::VISIBLE_STRING);
                CHECK(attr.value.stringValue == "TestManufacturer");
            }
            else if (attr.variation == 250)
            {
                foundProduct = true;
                CHECK(attr.value.type == opendnp3::DeviceAttrType::VISIBLE_STRING);
                CHECK(attr.value.stringValue == "InteropTestDevice");
            }
            else if (attr.variation == 248)
            {
                foundSerial = true;
                CHECK(attr.value.type == opendnp3::DeviceAttrType::VISIBLE_STRING);
                CHECK(attr.value.stringValue == "SN-12345");
            }
        }

        CHECK(foundManufacturer);
        CHECK(foundProduct);
        CHECK(foundSerial);
    }

    master->Disable();
}
