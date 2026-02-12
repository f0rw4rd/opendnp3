/*
 * Interoperability test fixture for opendnp3 vs stepfunc/dnp3.
 *
 * Provides helpers for starting stepfunc outstations/masters and connecting
 * opendnp3 components to them over TCP on ephemeral ports.
 */

#ifndef OPENDNP3_INTEROP_FIXTURE_H
#define OPENDNP3_INTEROP_FIXTURE_H

#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/app/DeviceAttributes.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/master/DefaultMasterApplication.h>
#include <opendnp3/master/ISOEHandler.h>
#include <opendnp3/outstation/DefaultOutstationApplication.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/outstation/UpdateBuilder.h>

#include <dnp3mocks/DatabaseHelpers.h>

#include <dnp3.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Utility: find an ephemeral port by binding to port 0, reading the assigned
// port, then closing the socket. There is a small TOCTOU window, but for
// tests this is acceptable.
// ---------------------------------------------------------------------------
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

inline uint16_t FindEphemeralPort()
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        throw std::runtime_error("socket() failed");
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        ::close(fd);
        throw std::runtime_error("bind() failed");
    }

    socklen_t len = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) < 0)
    {
        ::close(fd);
        throw std::runtime_error("getsockname() failed");
    }

    uint16_t port = ntohs(addr.sin_port);
    ::close(fd);
    return port;
}

inline std::string MakeEndpoint(uint16_t port)
{
    std::ostringstream oss;
    oss << "127.0.0.1:" << port;
    return oss.str();
}

// ---------------------------------------------------------------------------
// Minimal stepfunc outstation application / control handler implementations
// ---------------------------------------------------------------------------

class NullOutstationApplication : public dnp3::OutstationApplication
{
public:
    uint16_t get_processing_delay_ms() override
    {
        return 0;
    }

    dnp3::WriteTimeResult write_absolute_time(uint64_t /*time*/) override
    {
        return dnp3::WriteTimeResult::not_supported;
    }

    dnp3::ApplicationIin get_application_iin() override
    {
        return dnp3::ApplicationIin();
    }

    dnp3::RestartDelay cold_restart() override
    {
        return dnp3::RestartDelay::not_supported();
    }

    dnp3::RestartDelay warm_restart() override
    {
        return dnp3::RestartDelay::not_supported();
    }
};

class NullOutstationInformation : public dnp3::OutstationInformation
{
public:
    void process_request_from_idle(const dnp3::RequestHeader& /*header*/) override {}
    void broadcast_received(dnp3::FunctionCode /*function_code*/, dnp3::BroadcastAction /*action*/) override {}
    void enter_solicited_confirm_wait(uint8_t /*ecsn*/) override {}
    void solicited_confirm_timeout(uint8_t /*ecsn*/) override {}
    void solicited_confirm_received(uint8_t /*ecsn*/) override {}
    void solicited_confirm_wait_new_request() override {}
    void wrong_solicited_confirm_seq(uint8_t /*ecsn*/, uint8_t /*seq*/) override {}
    void unexpected_confirm(bool /*unsolicited*/, uint8_t /*seq*/) override {}
    void enter_unsolicited_confirm_wait(uint8_t /*ecsn*/) override {}
    void unsolicited_confirm_timeout(uint8_t /*ecsn*/, bool /*retry*/) override {}
    void unsolicited_confirmed(uint8_t /*ecsn*/) override {}
    void clear_restart_iin() override {}
};

class NullControlHandler : public dnp3::ControlHandler
{
public:
    void begin_fragment() override {}
    void end_fragment(dnp3::DatabaseHandle& /*database*/) override {}

    dnp3::CommandStatus select_g12v1(const dnp3::Group12Var1& /*value*/,
                                     uint16_t /*index*/,
                                     dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus operate_g12v1(const dnp3::Group12Var1& /*value*/,
                                      uint16_t /*index*/,
                                      dnp3::OperateType /*op*/,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus select_g41v1(int32_t /*value*/, uint16_t /*index*/, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus operate_g41v1(int32_t /*value*/,
                                      uint16_t /*index*/,
                                      dnp3::OperateType /*op*/,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus select_g41v2(int16_t /*value*/, uint16_t /*index*/, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus operate_g41v2(int16_t /*value*/,
                                      uint16_t /*index*/,
                                      dnp3::OperateType /*op*/,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus select_g41v3(float /*value*/, uint16_t /*index*/, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus operate_g41v3(float /*value*/,
                                      uint16_t /*index*/,
                                      dnp3::OperateType /*op*/,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus select_g41v4(double /*value*/, uint16_t /*index*/, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
    dnp3::CommandStatus operate_g41v4(double /*value*/,
                                      uint16_t /*index*/,
                                      dnp3::OperateType /*op*/,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::not_supported;
    }
};

// ---------------------------------------------------------------------------
// Synchronized read handler that collects received measurement data from
// the stepfunc master (ReadHandler) or opendnp3 master (ISOEHandler).
// ---------------------------------------------------------------------------

class CollectingReadHandler : public dnp3::ReadHandler
{
public:
    struct BinaryValue
    {
        uint16_t index;
        bool value;
    };
    struct DoubleBitBinaryValue
    {
        uint16_t index;
        dnp3::DoubleBit value;
    };
    struct AnalogValue
    {
        uint16_t index;
        double value;
    };
    struct CounterValue
    {
        uint16_t index;
        uint32_t value;
    };
    struct FrozenCounterValue
    {
        uint16_t index;
        uint32_t value;
    };
    struct FrozenAnalogValue
    {
        uint16_t index;
        double value;
    };
    struct BinaryOutputStatusValue
    {
        uint16_t index;
        bool value;
    };
    struct AnalogOutputStatusValue
    {
        uint16_t index;
        double value;
    };
    struct OctetStringValue
    {
        uint16_t index;
        std::vector<uint8_t> data;
    };
    struct StringAttrValue
    {
        uint8_t variation;
        std::string value;
    };

    std::mutex mutex;
    std::condition_variable cv;

    std::vector<BinaryValue> binaries;
    std::vector<DoubleBitBinaryValue> double_bit_binaries;
    std::vector<AnalogValue> analogs;
    std::vector<CounterValue> counters;
    std::vector<FrozenCounterValue> frozen_counters;
    std::vector<FrozenAnalogValue> frozen_analogs;
    std::vector<BinaryOutputStatusValue> binary_output_statuses;
    std::vector<AnalogOutputStatusValue> analog_output_statuses;
    std::vector<OctetStringValue> octet_strings;
    std::vector<StringAttrValue> string_attrs;
    uint32_t fragment_count = 0;

    void begin_fragment(dnp3::ReadType /*read_type*/, const dnp3::ResponseHeader& /*header*/) override {}

    void end_fragment(dnp3::ReadType /*read_type*/, const dnp3::ResponseHeader& /*header*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        ++fragment_count;
        cv.notify_all();
    }

    void handle_binary_input(const dnp3::HeaderInfo& /*info*/, dnp3::BinaryInputIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            binaries.push_back({v.index, v.value});
        }
    }

    void handle_double_bit_binary_input(const dnp3::HeaderInfo& /*info*/,
                                        dnp3::DoubleBitBinaryInputIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            double_bit_binaries.push_back({v.index, v.value});
        }
    }

    void handle_analog_input(const dnp3::HeaderInfo& /*info*/, dnp3::AnalogInputIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            analogs.push_back({v.index, v.value});
        }
    }

    void handle_counter(const dnp3::HeaderInfo& /*info*/, dnp3::CounterIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            counters.push_back({v.index, v.value});
        }
    }

    void handle_frozen_counter(const dnp3::HeaderInfo& /*info*/, dnp3::FrozenCounterIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            frozen_counters.push_back({v.index, v.value});
        }
    }

    void handle_frozen_analog_input(const dnp3::HeaderInfo& /*info*/, dnp3::FrozenAnalogInputIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            frozen_analogs.push_back({v.index, v.value});
        }
    }

    void handle_binary_output_status(const dnp3::HeaderInfo& /*info*/,
                                     dnp3::BinaryOutputStatusIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            binary_output_statuses.push_back({v.index, v.value});
        }
    }

    void handle_analog_output_status(const dnp3::HeaderInfo& /*info*/,
                                     dnp3::AnalogOutputStatusIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            analog_output_statuses.push_back({v.index, v.value});
        }
    }

    void handle_octet_string(const dnp3::HeaderInfo& /*info*/, dnp3::OctetStringIterator& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (values.next())
        {
            auto v = values.get();
            OctetStringValue osv;
            osv.index = v.index;
            while (v.value.next())
            {
                osv.data.push_back(v.value.get());
            }
            octet_strings.push_back(std::move(osv));
        }
    }

    void handle_string_attr(const dnp3::HeaderInfo& /*info*/,
                            dnp3::StringAttr /*attr*/,
                            uint8_t /*set*/,
                            uint8_t variation,
                            const char* value) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        string_attrs.push_back({variation, std::string(value)});
    }

    bool WaitForFragments(uint32_t count, std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return fragment_count >= count; });
    }
};

// ---------------------------------------------------------------------------
// Collecting ISOEHandler for opendnp3 master
// ---------------------------------------------------------------------------

class CollectingSOEHandler : public opendnp3::ISOEHandler
{
public:
    struct BinaryValue
    {
        uint16_t index;
        bool value;
    };
    struct DoubleBitBinaryValue
    {
        uint16_t index;
        opendnp3::DoubleBit value;
    };
    struct AnalogValue
    {
        uint16_t index;
        double value;
    };
    struct CounterValue
    {
        uint16_t index;
        uint32_t value;
    };
    struct FrozenCounterValue
    {
        uint16_t index;
        uint32_t value;
    };
    struct BinaryOutputStatusValue
    {
        uint16_t index;
        bool value;
    };
    struct AnalogOutputStatusValue
    {
        uint16_t index;
        double value;
    };
    struct OctetStringValue
    {
        uint16_t index;
        std::vector<uint8_t> data;
    };
    struct DeviceAttributeRecord
    {
        uint8_t set;
        uint8_t variation;
        opendnp3::DeviceAttributeValue value;
    };

    std::mutex mutex;
    std::condition_variable cv;

    std::vector<BinaryValue> binaries;
    std::vector<DoubleBitBinaryValue> double_bit_binaries;
    std::vector<AnalogValue> analogs;
    std::vector<CounterValue> counters;
    std::vector<FrozenCounterValue> frozen_counters;
    std::vector<BinaryOutputStatusValue> binary_output_statuses;
    std::vector<AnalogOutputStatusValue> analog_output_statuses;
    std::vector<OctetStringValue> octet_strings;
    std::vector<DeviceAttributeRecord> device_attributes;
    uint32_t fragment_count = 0;

    void BeginFragment(const opendnp3::ResponseInfo& /*info*/) override {}

    void EndFragment(const opendnp3::ResponseInfo& /*info*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        ++fragment_count;
        cv.notify_all();
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::Binary>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::Binary>& item) {
            binaries.push_back({item.index, item.value.value});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::DoubleBitBinary>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::DoubleBitBinary>& item) {
            double_bit_binaries.push_back({item.index, item.value.value});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::Analog>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::Analog>& item) {
            analogs.push_back({item.index, item.value.value});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::Counter>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::Counter>& item) {
            counters.push_back({item.index, static_cast<uint32_t>(item.value.value)});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::FrozenCounter>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::FrozenCounter>& item) {
            frozen_counters.push_back({item.index, static_cast<uint32_t>(item.value.value)});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::BinaryOutputStatus>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::BinaryOutputStatus>& item) {
            binary_output_statuses.push_back({item.index, item.value.value});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::AnalogOutputStatus>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::AnalogOutputStatus>& item) {
            analog_output_statuses.push_back({item.index, item.value.value});
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::DNPTime>& /*values*/) override
    {
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::OctetString>>& values) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        values.ForeachItem([&](const opendnp3::Indexed<opendnp3::OctetString>& item) {
            OctetStringValue v;
            v.index = item.index;
            auto buf = item.value.ToBuffer();
            v.data.assign(buf.data, buf.data + buf.length);
            octet_strings.push_back(std::move(v));
        });
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::BinaryCommandEvent>>& /*values*/) override
    {
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::AnalogCommandEvent>>& /*values*/) override
    {
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::TimeAndInterval>>& /*values*/) override
    {
    }

    void Process(const opendnp3::HeaderInfo& /*info*/,
                 const opendnp3::ICollection<opendnp3::Indexed<opendnp3::AnalogInputDeadband>>& /*values*/) override
    {
    }

    void OnDeviceAttribute(const opendnp3::HeaderInfo& /*info*/,
                           uint8_t set,
                           uint8_t variation,
                           const opendnp3::DeviceAttributeValue& value) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        device_attributes.push_back({set, variation, value});
    }

    bool WaitForFragments(uint32_t count, std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return fragment_count >= count; });
    }
};

// ---------------------------------------------------------------------------
// Null connection state listener for stepfunc outstation server
// ---------------------------------------------------------------------------

class NullConnectionStateListener : public dnp3::ConnectionStateListener
{
public:
    void on_change(dnp3::ConnectionState /*state*/) override {}
};

// ---------------------------------------------------------------------------
// Synchronizing callback for stepfunc master read operations
// ---------------------------------------------------------------------------

class SyncReadCallback : public dnp3::ReadTaskCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    dnp3::ReadError result = dnp3::ReadError::shutdown;

    void on_complete(dnp3::Nothing /*result*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        result = dnp3::ReadError::ok;
        completed = true;
        cv.notify_all();
    }

    void on_failure(dnp3::ReadError error) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        result = error;
        completed = true;
        cv.notify_all();
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// Null stepfunc AssociationInformation
// ---------------------------------------------------------------------------

class NullAssociationInfo : public dnp3::AssociationInformation
{
public:
    void task_start(dnp3::TaskType, dnp3::FunctionCode, uint8_t) override {}
    void task_success(dnp3::TaskType, dnp3::FunctionCode, uint8_t) override {}
    void task_fail(dnp3::TaskType, dnp3::TaskError) override {}
    void unsolicited_response(bool, uint8_t) override {}
};

// ---------------------------------------------------------------------------
// Null stepfunc ClientStateListener
// ---------------------------------------------------------------------------

class NullClientStateListener : public dnp3::ClientStateListener
{
public:
    void on_change(dnp3::ClientState /*state*/) override {}
};

// ---------------------------------------------------------------------------
// Collecting stepfunc ControlHandler that accepts commands and records them
// ---------------------------------------------------------------------------

class CollectingControlHandler : public dnp3::ControlHandler
{
public:
    struct CROBRecord
    {
        uint16_t index;
        dnp3::OperateType op_type;
        uint8_t count;
        uint32_t on_time;
        uint32_t off_time;
    };
    struct AnalogRecord
    {
        uint16_t index;
        double value;
        int variation; // 1=g41v1(i32), 2=g41v2(i16), 3=g41v3(f32), 4=g41v4(f64)
        dnp3::OperateType op_type;
    };

    std::mutex mutex;
    std::condition_variable cv;
    std::vector<CROBRecord> crobs;
    std::vector<AnalogRecord> analogs;
    uint32_t operate_count = 0;

    void begin_fragment() override {}
    void end_fragment(dnp3::DatabaseHandle& /*database*/) override {}

    dnp3::CommandStatus select_g12v1(const dnp3::Group12Var1& /*value*/,
                                     uint16_t /*index*/,
                                     dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus operate_g12v1(const dnp3::Group12Var1& value,
                                      uint16_t index,
                                      dnp3::OperateType op,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        crobs.push_back({index, op, value.count, value.on_time, value.off_time});
        ++operate_count;
        cv.notify_all();
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus select_g41v1(int32_t /*value*/, uint16_t /*index*/, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus operate_g41v1(int32_t value,
                                      uint16_t index,
                                      dnp3::OperateType op,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        analogs.push_back({index, static_cast<double>(value), 1, op});
        ++operate_count;
        cv.notify_all();
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus select_g41v2(int16_t /*value*/, uint16_t /*index*/, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus operate_g41v2(int16_t value,
                                      uint16_t index,
                                      dnp3::OperateType op,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        analogs.push_back({index, static_cast<double>(value), 2, op});
        ++operate_count;
        cv.notify_all();
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus select_g41v3(float value, uint16_t index, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus operate_g41v3(float value,
                                      uint16_t index,
                                      dnp3::OperateType op,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        analogs.push_back({index, static_cast<double>(value), 3, op});
        ++operate_count;
        cv.notify_all();
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus select_g41v4(double value, uint16_t index, dnp3::DatabaseHandle& /*db*/) override
    {
        return dnp3::CommandStatus::success;
    }
    dnp3::CommandStatus operate_g41v4(double value,
                                      uint16_t index,
                                      dnp3::OperateType op,
                                      dnp3::DatabaseHandle& /*db*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        analogs.push_back({index, value, 4, op});
        ++operate_count;
        cv.notify_all();
        return dnp3::CommandStatus::success;
    }

    bool WaitForOperates(uint32_t count, std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return operate_count >= count; });
    }
};

// ---------------------------------------------------------------------------
// Synchronizing command callback for opendnp3 master command operations
// ---------------------------------------------------------------------------

class SyncCommandCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::TaskCompletion summary = opendnp3::TaskCompletion::FAILURE_NO_COMMS;
    std::vector<opendnp3::CommandPointResult> results;

    opendnp3::CommandResultCallbackT Callback()
    {
        return [this](const opendnp3::ICommandTaskResult& result) {
            std::lock_guard<std::mutex> lock(mutex);
            summary = result.summary;
            result.ForeachItem([this](const opendnp3::CommandPointResult& r) { results.push_back(r); });
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// Timeout constant used throughout the tests
// ---------------------------------------------------------------------------
static constexpr auto INTEROP_TIMEOUT = std::chrono::seconds(10);

#endif // OPENDNP3_INTEROP_FIXTURE_H
