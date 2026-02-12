/*
 * Interop tests: opendnp3 master vs FreyrSCADA outstation.
 *
 * Tests exercise opendnp3 acting as master (TCP client) connecting to
 * a FreyrSCADA server (outstation) on TCP localhost.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "FreyrWrapper.h"

#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/master/DefaultMasterApplication.h>
#include <opendnp3/master/ISOEHandler.h>
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
// Collecting SOE handler for opendnp3 master
// ---------------------------------------------------------------
class CollectingSOEHandler final : public ISOEHandler
{
public:
    struct BinaryRecord
    {
        uint16_t index;
        bool value;
    };
    struct AnalogRecord
    {
        uint16_t index;
        double value;
    };
    struct BinaryOutputRecord
    {
        uint16_t index;
        bool value;
    };
    struct AnalogOutputRecord
    {
        uint16_t index;
        double value;
    };

    std::mutex mutex;
    std::condition_variable cv;
    std::vector<BinaryRecord> binaries;
    std::vector<AnalogRecord> analogs;
    std::vector<BinaryOutputRecord> binaryOutputs;
    std::vector<AnalogOutputRecord> analogOutputs;
    std::atomic<int> fragmentCount{0};

    void BeginFragment(const ResponseInfo& info) override {}

    void EndFragment(const ResponseInfo& info) override
    {
        fragmentCount.fetch_add(1);
        cv.notify_all();
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) override
    {
        std::lock_guard<std::mutex> lk(mutex);
        values.ForeachItem([&](const Indexed<Binary>& item) { binaries.push_back({item.index, item.value.value}); });
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values) override
    {
        std::lock_guard<std::mutex> lk(mutex);
        values.ForeachItem([&](const Indexed<Analog>& item) { analogs.push_back({item.index, item.value.value}); });
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) override
    {
        std::lock_guard<std::mutex> lk(mutex);
        values.ForeachItem(
            [&](const Indexed<BinaryOutputStatus>& item) { binaryOutputs.push_back({item.index, item.value.value}); });
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) override
    {
        std::lock_guard<std::mutex> lk(mutex);
        values.ForeachItem(
            [&](const Indexed<AnalogOutputStatus>& item) { analogOutputs.push_back({item.index, item.value.value}); });
    }

    // Unused process methods
    void Process(const HeaderInfo&, const ICollection<Indexed<DoubleBitBinary>>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<Counter>>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<FrozenCounter>>&) override {}
    void Process(const HeaderInfo&, const ICollection<DNPTime>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<OctetString>>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<BinaryCommandEvent>>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<AnalogCommandEvent>>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<TimeAndInterval>>&) override {}
    void Process(const HeaderInfo&, const ICollection<Indexed<AnalogInputDeadband>>&) override {}

    bool WaitForFragments(int count, std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lk(mutex);
        return cv.wait_for(lk, timeout, [&] { return fragmentCount.load() >= count; });
    }

    void Reset()
    {
        std::lock_guard<std::mutex> lk(mutex);
        binaries.clear();
        analogs.clear();
        binaryOutputs.clear();
        analogOutputs.clear();
        fragmentCount.store(0);
    }
};

// ---------------------------------------------------------------
// Channel listener for waiting until connection is established
// ---------------------------------------------------------------
class WaitableChannelListener : public IChannelListener
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
static constexpr uint16_t FREYR_PORT = 21000;
static constexpr uint16_t SLAVE_ADDR = 1;
static constexpr uint16_t MASTER_ADDR = 2;
static constexpr uint16_t NUM_POINTS = 10;
static const auto TEST_TIMEOUT = std::chrono::seconds(15);
static const auto CONNECT_TIMEOUT = std::chrono::seconds(10);

#define SUITE(name) "FreyrInterop - OpendnpMaster - " name

// ---------------------------------------------------------------
// Test: opendnp3 master integrity poll against FreyrSCADA server
// ---------------------------------------------------------------
TEST_CASE(SUITE("IntegrityPoll"))
{
    // Start FreyrSCADA server
    FreyrServer::Config srvCfg;
    srvCfg.tcpPort = FREYR_PORT;
    srvCfg.slaveAddress = SLAVE_ADDR;
    srvCfg.masterAddress = MASTER_ADDR;
    srvCfg.numBinaryInputs = NUM_POINTS;
    srvCfg.numAnalogInputs = NUM_POINTS;
    srvCfg.numBinaryOutputs = NUM_POINTS;
    srvCfg.numAnalogOutputs = NUM_POINTS;

    FreyrServer server(srvCfg);
    server.Start();

    // Set some known values
    server.UpdateBinaryInput(0, true);
    server.UpdateBinaryInput(1, false);
    server.UpdateAnalogInput(0, 42.5f);
    server.UpdateAnalogInput(1, -17.3f);

    // Give the server a moment to start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start opendnp3 master
    const auto LOG_LEVELS = levels::NOTHING;
    DNP3Manager manager(2);

    auto listener = std::make_shared<WaitableChannelListener>();
    auto channel = manager.AddTCPClient("interop-master", LOG_LEVELS, ChannelRetry::Default(),
                                        {IPEndpoint("127.0.0.1", FREYR_PORT)}, "127.0.0.1", listener);

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = ClassField::AllClasses();
    masterCfg.master.unsolClassMask = ClassField::None();
    masterCfg.link.LocalAddr = MASTER_ADDR;
    masterCfg.link.RemoteAddr = SLAVE_ADDR;

    auto master = channel->AddMaster("master", soeHandler, DefaultMasterApplication::Create(), masterCfg);
    master->Enable();

    // Wait for connection
    REQUIRE(listener->WaitForOpen(CONNECT_TIMEOUT));

    // Wait for integrity poll response fragments
    REQUIRE(soeHandler->WaitForFragments(1, TEST_TIMEOUT));

    // Check received binary inputs
    {
        std::lock_guard<std::mutex> lk(soeHandler->mutex);
        // We should have received BI data
        bool foundBI0 = false;
        for (const auto& rec : soeHandler->binaries)
        {
            if (rec.index == 0 && rec.value == true)
                foundBI0 = true;
        }
        CHECK(foundBI0);
    }

    // Check received analog inputs
    {
        std::lock_guard<std::mutex> lk(soeHandler->mutex);
        bool foundAI0 = false;
        for (const auto& rec : soeHandler->analogs)
        {
            if (rec.index == 0 && std::abs(rec.value - 42.5) < 1.0)
                foundAI0 = true;
        }
        CHECK(foundAI0);
    }

    // Cleanup
    master->Disable();
    server.Stop();
}

// ---------------------------------------------------------------
// Test: opendnp3 master reads updated values
// ---------------------------------------------------------------
TEST_CASE(SUITE("ReadUpdatedValues"))
{
    FreyrServer::Config srvCfg;
    srvCfg.tcpPort = FREYR_PORT + 1;
    srvCfg.slaveAddress = SLAVE_ADDR;
    srvCfg.masterAddress = MASTER_ADDR;
    srvCfg.numBinaryInputs = NUM_POINTS;
    srvCfg.numAnalogInputs = NUM_POINTS;
    srvCfg.numBinaryOutputs = NUM_POINTS;
    srvCfg.numAnalogOutputs = NUM_POINTS;

    FreyrServer server(srvCfg);
    server.Start();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    DNP3Manager manager(2);
    auto listener = std::make_shared<WaitableChannelListener>();
    auto channel = manager.AddTCPClient("interop-master-2", levels::NOTHING, ChannelRetry::Default(),
                                        {IPEndpoint("127.0.0.1", FREYR_PORT + 1)}, "127.0.0.1", listener);

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = ClassField::AllClasses();
    masterCfg.master.unsolClassMask = ClassField::None();
    masterCfg.master.integrityOnEventOverflowIIN = false;
    masterCfg.link.LocalAddr = MASTER_ADDR;
    masterCfg.link.RemoteAddr = SLAVE_ADDR;

    auto master = channel->AddMaster("master2", soeHandler, DefaultMasterApplication::Create(), masterCfg);
    master->Enable();
    REQUIRE(listener->WaitForOpen(CONNECT_TIMEOUT));

    // Wait for initial integrity poll
    REQUIRE(soeHandler->WaitForFragments(1, TEST_TIMEOUT));

    // Now update a value on the server
    server.UpdateAnalogInput(5, 123.456f);

    // Trigger another integrity poll
    soeHandler->Reset();
    auto scan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Seconds(2), soeHandler);
    REQUIRE(soeHandler->WaitForFragments(1, TEST_TIMEOUT));

    // Verify the value came through
    {
        std::lock_guard<std::mutex> lk(soeHandler->mutex);
        bool found = false;
        for (const auto& rec : soeHandler->analogs)
        {
            if (rec.index == 5 && std::abs(rec.value - 123.456) < 1.0)
                found = true;
        }
        CHECK(found);
    }

    master->Disable();
    server.Stop();
}

// ---------------------------------------------------------------
// Test: opendnp3 master sends CROB to FreyrSCADA outstation
// ---------------------------------------------------------------
TEST_CASE(SUITE("DirectOperateCROB"))
{
    FreyrServer::operateCallCount.store(0);

    FreyrServer::Config srvCfg;
    srvCfg.tcpPort = FREYR_PORT + 2;
    srvCfg.slaveAddress = SLAVE_ADDR;
    srvCfg.masterAddress = MASTER_ADDR;
    srvCfg.numBinaryInputs = NUM_POINTS;
    srvCfg.numAnalogInputs = NUM_POINTS;
    srvCfg.numBinaryOutputs = NUM_POINTS;
    srvCfg.numAnalogOutputs = NUM_POINTS;

    FreyrServer server(srvCfg);
    server.Start();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    DNP3Manager manager(2);
    auto listener = std::make_shared<WaitableChannelListener>();
    auto channel = manager.AddTCPClient("interop-master-3", levels::NOTHING, ChannelRetry::Default(),
                                        {IPEndpoint("127.0.0.1", FREYR_PORT + 2)}, "127.0.0.1", listener);

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = ClassField::AllClasses();
    masterCfg.link.LocalAddr = MASTER_ADDR;
    masterCfg.link.RemoteAddr = SLAVE_ADDR;

    auto master = channel->AddMaster("master3", soeHandler, DefaultMasterApplication::Create(), masterCfg);
    master->Enable();
    REQUIRE(listener->WaitForOpen(CONNECT_TIMEOUT));
    REQUIRE(soeHandler->WaitForFragments(1, TEST_TIMEOUT));

    // Send DirectOperate CROB on index 0
    ControlRelayOutputBlock crob(OperationType::LATCH_ON);
    auto cmdCallback = [](const ICommandTaskResult& result) {
        // We just need to verify it was sent
    };
    master->DirectOperate(CommandSet({WithIndex(crob, 0)}), cmdCallback);

    // Wait for the operate callback on the FreyrSCADA side
    auto deadline = std::chrono::steady_clock::now() + TEST_TIMEOUT;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (FreyrServer::operateCallCount.load() > 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK(FreyrServer::operateCallCount.load() > 0);

    master->Disable();
    server.Stop();
}

// ---------------------------------------------------------------
// Test: opendnp3 master sends AnalogOutput to FreyrSCADA outstation
// ---------------------------------------------------------------
TEST_CASE(SUITE("DirectOperateAnalogOutput"))
{
    FreyrServer::operateCallCount.store(0);

    FreyrServer::Config srvCfg;
    srvCfg.tcpPort = FREYR_PORT + 3;
    srvCfg.slaveAddress = SLAVE_ADDR;
    srvCfg.masterAddress = MASTER_ADDR;
    srvCfg.numBinaryInputs = NUM_POINTS;
    srvCfg.numAnalogInputs = NUM_POINTS;
    srvCfg.numBinaryOutputs = NUM_POINTS;
    srvCfg.numAnalogOutputs = NUM_POINTS;

    FreyrServer server(srvCfg);
    server.Start();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    DNP3Manager manager(2);
    auto listener = std::make_shared<WaitableChannelListener>();
    auto channel = manager.AddTCPClient("interop-master-4", levels::NOTHING, ChannelRetry::Default(),
                                        {IPEndpoint("127.0.0.1", FREYR_PORT + 3)}, "127.0.0.1", listener);

    auto soeHandler = std::make_shared<CollectingSOEHandler>();

    MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = ClassField::AllClasses();
    masterCfg.link.LocalAddr = MASTER_ADDR;
    masterCfg.link.RemoteAddr = SLAVE_ADDR;

    auto master = channel->AddMaster("master4", soeHandler, DefaultMasterApplication::Create(), masterCfg);
    master->Enable();
    REQUIRE(listener->WaitForOpen(CONNECT_TIMEOUT));
    REQUIRE(soeHandler->WaitForFragments(1, TEST_TIMEOUT));

    // Send AnalogOutputFloat32 on index 3
    AnalogOutputFloat32 aof(99.5f);
    auto cmdCallback = [](const ICommandTaskResult& result) {};
    master->DirectOperate(CommandSet({WithIndex(aof, 3)}), cmdCallback);

    // Wait for the operate callback on the FreyrSCADA side
    auto deadline = std::chrono::steady_clock::now() + TEST_TIMEOUT;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (FreyrServer::operateCallCount.load() > 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK(FreyrServer::operateCallCount.load() > 0);

    master->Disable();
    server.Stop();
}
