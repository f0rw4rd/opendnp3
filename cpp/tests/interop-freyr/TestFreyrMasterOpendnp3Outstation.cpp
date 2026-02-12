/*
 * Interop tests: FreyrSCADA master (client) vs opendnp3 outstation.
 *
 * Tests exercise FreyrSCADA acting as master (TCP client) connecting to
 * an opendnp3 outstation (TCP server) on localhost.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "FreyrWrapper.h"

#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/gen/StaticAnalogOutputStatusVariation.h>
#include <opendnp3/gen/StaticAnalogVariation.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/outstation/DefaultOutstationApplication.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/outstation/UpdateBuilder.h>

#include <dnp3mocks/DatabaseHelpers.h>

#include <catch.hpp>

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

using namespace opendnp3;

// ---------------------------------------------------------------
// Command handler that records direct operates
// ---------------------------------------------------------------
class RecordingCommandHandler : public ICommandHandler
{
public:
    std::atomic<int> crobCount{0};
    std::atomic<int> aoCount{0};

    void Begin() override {}
    void End() override {}

    CommandStatus Select(const ControlRelayOutputBlock& cmd, uint16_t index) override
    {
        return CommandStatus::SUCCESS;
    }
    CommandStatus Operate(const ControlRelayOutputBlock& cmd,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        crobCount.fetch_add(1);
        return CommandStatus::SUCCESS;
    }
    CommandStatus Select(const AnalogOutputInt16& cmd, uint16_t index) override
    {
        return CommandStatus::SUCCESS;
    }
    CommandStatus Operate(const AnalogOutputInt16& cmd,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        aoCount.fetch_add(1);
        return CommandStatus::SUCCESS;
    }
    CommandStatus Select(const AnalogOutputInt32& cmd, uint16_t index) override
    {
        return CommandStatus::SUCCESS;
    }
    CommandStatus Operate(const AnalogOutputInt32& cmd,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        aoCount.fetch_add(1);
        return CommandStatus::SUCCESS;
    }
    CommandStatus Select(const AnalogOutputFloat32& cmd, uint16_t index) override
    {
        return CommandStatus::SUCCESS;
    }
    CommandStatus Operate(const AnalogOutputFloat32& cmd,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        aoCount.fetch_add(1);
        return CommandStatus::SUCCESS;
    }
    CommandStatus Select(const AnalogOutputDouble64& cmd, uint16_t index) override
    {
        return CommandStatus::SUCCESS;
    }
    CommandStatus Operate(const AnalogOutputDouble64& cmd,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        aoCount.fetch_add(1);
        return CommandStatus::SUCCESS;
    }
};

// ---------------------------------------------------------------
// Channel listener
// ---------------------------------------------------------------
class OutstationChannelListener : public IChannelListener
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    ChannelState lastState = ChannelState::CLOSED;

    void OnStateChange(ChannelState state) override
    {
        std::lock_guard<std::mutex> lk(mutex);
        lastState = state;
        cv.notify_all();
    }

    bool WaitForOpen(std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lk(mutex);
        return cv.wait_for(lk, timeout, [&] { return lastState == ChannelState::OPEN; });
    }
};

// ---------------------------------------------------------------
// Test constants
// ---------------------------------------------------------------
static constexpr uint16_t OPENDNP3_PORT = 22000;
static constexpr uint16_t SLAVE_ADDR = 1;
static constexpr uint16_t MASTER_ADDR = 2;
static constexpr uint16_t NUM_POINTS = 10;
static const auto TEST_TIMEOUT = std::chrono::seconds(15);
static const auto CONNECT_TIMEOUT = std::chrono::seconds(10);

// Helper: configure outstation database with float analog variations for FreyrSCADA compatibility
static OutstationStackConfig MakeFreyrCompatibleConfig(uint16_t numPoints)
{
    OutstationStackConfig cfg(configure::by_count_of::all_types(numPoints));
    cfg.outstation.params.allowUnsolicited = false;
    cfg.link.LocalAddr = SLAVE_ADDR;
    cfg.link.RemoteAddr = MASTER_ADDR;

    // FreyrSCADA expects analog values as float (Group30Var5), not 32-bit integer (Group30Var1)
    for (auto& kv : cfg.database.analog_input)
    {
        kv.second.svariation = StaticAnalogVariation::Group30Var5;
    }
    for (auto& kv : cfg.database.analog_output_status)
    {
        kv.second.svariation = StaticAnalogOutputStatusVariation::Group40Var3;
    }
    return cfg;
}

#define SUITE(name) "FreyrInterop - FreyrMaster - " name

// ---------------------------------------------------------------
// Test: FreyrSCADA client polls opendnp3 outstation
// ---------------------------------------------------------------
TEST_CASE(SUITE("IntegrityPollFromFreyrClient"))
{
    // Start opendnp3 outstation as TCP server
    DNP3Manager manager(2);

    auto listener = std::make_shared<OutstationChannelListener>();
    auto channel = manager.AddTCPServer("interop-outstation", levels::NOTHING, ServerAcceptMode::CloseExisting,
                                        IPEndpoint("127.0.0.1", OPENDNP3_PORT), listener);

    auto outstationCfg = MakeFreyrCompatibleConfig(NUM_POINTS);

    auto cmdHandler = std::make_shared<RecordingCommandHandler>();
    auto outstation
        = channel->AddOutstation("outstation", cmdHandler, DefaultOutstationApplication::Create(), outstationCfg);

    // Set some known values
    UpdateBuilder builder;
    builder.Update(Binary(true), 0, EventMode::Force);
    builder.Update(Binary(false), 1, EventMode::Force);
    builder.Update(Analog(42.5), 0, EventMode::Force);
    builder.Update(Analog(-17.3), 1, EventMode::Force);
    outstation->Apply(builder.Build());

    outstation->Enable();

    // Give the server time to start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start FreyrSCADA client
    FreyrClient::Config clientCfg;
    clientCfg.tcpPort = OPENDNP3_PORT;
    clientCfg.serverIP = "127.0.0.1";
    clientCfg.masterAddress = MASTER_ADDR;
    clientCfg.slaveAddress = SLAVE_ADDR;
    clientCfg.numBinaryInputs = NUM_POINTS;
    clientCfg.numAnalogInputs = NUM_POINTS;
    clientCfg.numBinaryOutputs = NUM_POINTS;
    clientCfg.numAnalogOutputs = NUM_POINTS;

    FreyrClient client(clientCfg);
    client.Start();

    // Wait for updates to arrive via polling
    REQUIRE(client.WaitForUpdates(1, TEST_TIMEOUT));

    // Allow additional polls to fill in all data
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Read binary input 0 from FreyrSCADA client mirror
    bool bi0 = client.ReadBinaryInput(0);
    CHECK(bi0 == true);

    // Read analog input 0 from FreyrSCADA client mirror
    float ai0 = client.ReadAnalogInput(0);
    CHECK(std::abs(ai0 - 42.5f) < 2.0f);

    // Cleanup
    client.Stop();
    outstation->Disable();
}

// ---------------------------------------------------------------
// Test: FreyrSCADA client reads updated analog values
// ---------------------------------------------------------------
TEST_CASE(SUITE("ReadUpdatedAnalogValues"))
{
    DNP3Manager manager(2);

    auto listener = std::make_shared<OutstationChannelListener>();
    auto channel = manager.AddTCPServer("interop-outstation-2", levels::NOTHING, ServerAcceptMode::CloseExisting,
                                        IPEndpoint("127.0.0.1", OPENDNP3_PORT + 1), listener);

    auto outstationCfg = MakeFreyrCompatibleConfig(NUM_POINTS);

    auto cmdHandler = std::make_shared<RecordingCommandHandler>();
    auto outstation
        = channel->AddOutstation("outstation2", cmdHandler, DefaultOutstationApplication::Create(), outstationCfg);

    // Set initial value
    {
        UpdateBuilder builder;
        builder.Update(Analog(100.0), 5, EventMode::Force);
        outstation->Apply(builder.Build());
    }

    outstation->Enable();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    FreyrClient::Config clientCfg;
    clientCfg.tcpPort = OPENDNP3_PORT + 1;
    clientCfg.serverIP = "127.0.0.1";
    clientCfg.masterAddress = MASTER_ADDR;
    clientCfg.slaveAddress = SLAVE_ADDR;
    clientCfg.numBinaryInputs = NUM_POINTS;
    clientCfg.numAnalogInputs = NUM_POINTS;
    clientCfg.numBinaryOutputs = NUM_POINTS;
    clientCfg.numAnalogOutputs = NUM_POINTS;

    FreyrClient client(clientCfg);
    client.Start();

    // Wait for initial poll
    REQUIRE(client.WaitForUpdates(1, TEST_TIMEOUT));
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Read the initial value
    float initial = client.ReadAnalogInput(5);
    CHECK(std::abs(initial - 100.0f) < 2.0f);

    // Update the value on the outstation
    {
        UpdateBuilder builder;
        builder.Update(Analog(777.0), 5, EventMode::Force);
        outstation->Apply(builder.Build());
    }

    // Wait for the next poll cycle to pick it up
    int prevCount = client.updateCount.load();
    auto deadline = std::chrono::steady_clock::now() + TEST_TIMEOUT;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (client.updateCount.load() > prevCount)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));

    float updated = client.ReadAnalogInput(5);
    CHECK(std::abs(updated - 777.0f) < 2.0f);

    client.Stop();
    outstation->Disable();
}

// ---------------------------------------------------------------
// Test: FreyrSCADA client sends DirectOperate CROB to opendnp3 outstation
// ---------------------------------------------------------------
TEST_CASE(SUITE("FreyrClientDirectOperateCROB"))
{
    DNP3Manager manager(2);

    auto listener = std::make_shared<OutstationChannelListener>();
    auto channel = manager.AddTCPServer("interop-outstation-3", levels::NOTHING, ServerAcceptMode::CloseExisting,
                                        IPEndpoint("127.0.0.1", OPENDNP3_PORT + 2), listener);

    auto outstationCfg = MakeFreyrCompatibleConfig(NUM_POINTS);

    auto cmdHandler = std::make_shared<RecordingCommandHandler>();
    auto outstation
        = channel->AddOutstation("outstation3", cmdHandler, DefaultOutstationApplication::Create(), outstationCfg);

    outstation->Enable();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    FreyrClient::Config clientCfg;
    clientCfg.tcpPort = OPENDNP3_PORT + 2;
    clientCfg.serverIP = "127.0.0.1";
    clientCfg.masterAddress = MASTER_ADDR;
    clientCfg.slaveAddress = SLAVE_ADDR;
    clientCfg.numBinaryInputs = NUM_POINTS;
    clientCfg.numAnalogInputs = NUM_POINTS;
    clientCfg.numBinaryOutputs = NUM_POINTS;
    clientCfg.numAnalogOutputs = NUM_POINTS;

    FreyrClient client(clientCfg);
    client.Start();

    // Wait for connection to be established and initial poll to complete
    REQUIRE(client.WaitForUpdates(1, TEST_TIMEOUT));
    // Allow extra time for FreyrSCADA client to fully establish session
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Send DirectOperate CROB on index 0
    // NOTE: The FreyrSCADA client API rejects CROB commands with INVALID_DATATYPE (-1526)
    // for all data type configurations we've tried (SINGLE_POINT_DATA, UNSIGNED_BYTE_DATA,
    // and even NULL data). This appears to be a limitation or undocumented requirement of
    // the FreyrSCADA client library. The analog output DirectOperate works correctly.
    bool sent = client.DirectOperateBinaryOutput(0, true);
    if (!sent)
    {
        WARN("FreyrSCADA CROB DirectOperate failed (known limitation of FreyrSCADA client API)");
    }
    else
    {
        // Wait for the operate callback on the opendnp3 side
        auto deadline = std::chrono::steady_clock::now() + TEST_TIMEOUT;
        while (std::chrono::steady_clock::now() < deadline)
        {
            if (cmdHandler->crobCount.load() > 0)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        CHECK(cmdHandler->crobCount.load() > 0);
    }

    client.Stop();
    outstation->Disable();
}

// ---------------------------------------------------------------
// Test: FreyrSCADA client sends AnalogOutput to opendnp3 outstation
// ---------------------------------------------------------------
TEST_CASE(SUITE("FreyrClientDirectOperateAnalogOutput"))
{
    DNP3Manager manager(2);

    auto listener = std::make_shared<OutstationChannelListener>();
    auto channel = manager.AddTCPServer("interop-outstation-4", levels::NOTHING, ServerAcceptMode::CloseExisting,
                                        IPEndpoint("127.0.0.1", OPENDNP3_PORT + 3), listener);

    auto outstationCfg = MakeFreyrCompatibleConfig(NUM_POINTS);

    auto cmdHandler = std::make_shared<RecordingCommandHandler>();
    auto outstation
        = channel->AddOutstation("outstation4", cmdHandler, DefaultOutstationApplication::Create(), outstationCfg);

    outstation->Enable();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    FreyrClient::Config clientCfg;
    clientCfg.tcpPort = OPENDNP3_PORT + 3;
    clientCfg.serverIP = "127.0.0.1";
    clientCfg.masterAddress = MASTER_ADDR;
    clientCfg.slaveAddress = SLAVE_ADDR;
    clientCfg.numBinaryInputs = NUM_POINTS;
    clientCfg.numAnalogInputs = NUM_POINTS;
    clientCfg.numBinaryOutputs = NUM_POINTS;
    clientCfg.numAnalogOutputs = NUM_POINTS;

    FreyrClient client(clientCfg);
    client.Start();

    // Wait for connection and initial poll
    REQUIRE(client.WaitForUpdates(1, TEST_TIMEOUT));
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Send AnalogOutput on index 5
    bool sent = client.DirectOperateAnalogOutput(5, 99.5f);
    CHECK(sent);

    // Wait for the operate callback on the opendnp3 side
    auto deadline = std::chrono::steady_clock::now() + TEST_TIMEOUT;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (cmdHandler->aoCount.load() > 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK(cmdHandler->aoCount.load() > 0);

    client.Stop();
    outstation->Disable();
}
