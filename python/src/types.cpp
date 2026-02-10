/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * This file is part of an experimental fork of opendnp3.
 * See the NOTICE file for upstream copyright attribution.
 */

#include "opendnp3/app/AnalogCommandEvent.h"
#include "opendnp3/app/AnalogOutput.h"
#include "opendnp3/app/BaseMeasurementTypes.h"
#include "opendnp3/app/BinaryCommandEvent.h"
#include "opendnp3/app/ClassField.h"
#include "opendnp3/app/ControlRelayOutputBlock.h"
#include "opendnp3/app/DNPTime.h"
#include "opendnp3/app/Flags.h"
#include "opendnp3/app/GroupVariationID.h"
#include "opendnp3/app/IINField.h"
#include "opendnp3/app/Indexed.h"
#include "opendnp3/app/MeasurementTypes.h"
#include "opendnp3/app/OctetString.h"
#include "opendnp3/channel/ChannelRetry.h"
#include "opendnp3/channel/IPEndpoint.h"
#include "opendnp3/channel/SerialSettings.h"
#include "opendnp3/link/LinkConfig.h"
#include "opendnp3/logging/ILogHandler.h"
#include "opendnp3/logging/LogLevels.h"
#include "opendnp3/master/CommandPointResult.h"
#include "opendnp3/master/FileOperationResult.h"
#include "opendnp3/master/HeaderInfo.h"
#include "opendnp3/master/HeaderTypes.h"
#include "opendnp3/master/MasterParams.h"
#include "opendnp3/master/MasterStackConfig.h"
#include "opendnp3/master/RestartOperationResult.h"
#include "opendnp3/master/TaskId.h"
#include "opendnp3/master/TaskInfo.h"
#include "opendnp3/outstation/ApplicationIIN.h"
#include "opendnp3/outstation/DatabaseConfig.h"
#include "opendnp3/outstation/EventBufferConfig.h"
#include "opendnp3/outstation/IFileHandler.h"
#include "opendnp3/outstation/IUpdateHandler.h"
#include "opendnp3/outstation/OutstationConfig.h"
#include "opendnp3/outstation/OutstationParams.h"
#include "opendnp3/outstation/OutstationStackConfig.h"
#include "opendnp3/outstation/UpdateBuilder.h"
#include "opendnp3/util/TimeDuration.h"
#include "opendnp3/util/UTCTimestamp.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace opendnp3;

void init_types(py::module_& m)
{
    // UTCTimestamp
    py::class_<UTCTimestamp>(m, "UTCTimestamp", "Strong typing for UTC timestamps")
        .def(py::init<>())
        .def(py::init<uint64_t>(), py::arg("msSinceEpoch"))
        .def_readwrite("msSinceEpoch", &UTCTimestamp::msSinceEpoch);

    // GroupVariationID
    py::class_<GroupVariationID>(m, "GroupVariationID", "Simple group/variation tuple")
        .def(py::init<>())
        .def(py::init<uint8_t, uint8_t>(), py::arg("group"), py::arg("variation"))
        .def_readwrite("group", &GroupVariationID::group)
        .def_readwrite("variation", &GroupVariationID::variation);

    // ClassField
    py::class_<ClassField>(m, "ClassField", "Specifies a set of event classes")
        .def(py::init<>())
        .def(py::init<bool, bool, bool, bool>(), py::arg("class0"), py::arg("class1"), py::arg("class2"),
             py::arg("class3"))
        .def_static("AllClasses", &ClassField::AllClasses)
        .def_static("AllEventClasses", &ClassField::AllEventClasses)
        .def_static("None_", &ClassField::None)
        .def("HasClass0", &ClassField::HasClass0)
        .def("HasClass1", &ClassField::HasClass1)
        .def("HasClass2", &ClassField::HasClass2)
        .def("HasClass3", &ClassField::HasClass3);

    // Header
    py::class_<Header>(m, "Header", "Class used to specify a header type for requests")
        .def(py::init<>())
        .def_static("AllObjects", &Header::AllObjects, py::arg("group"), py::arg("variation"))
        .def_static("From", &Header::From, py::arg("pc"))
        .def_static("Range8", &Header::Range8, py::arg("group"), py::arg("variation"), py::arg("start"),
                    py::arg("stop"))
        .def_static("Range16", &Header::Range16, py::arg("group"), py::arg("variation"), py::arg("start"),
                    py::arg("stop"))
        .def_static("Count8", &Header::Count8, py::arg("group"), py::arg("variation"), py::arg("count"))
        .def_static("Count16", &Header::Count16, py::arg("group"), py::arg("variation"), py::arg("count"))
        .def_static(
            "Raw",
            [](py::bytes data) {
                std::string s = data;
                return Header::Raw(reinterpret_cast<const uint8_t*>(s.data()), s.size());
            },
            py::arg("data"), "Create a header with raw bytes (group/var/qualifier + object data)");

    // IINField
    py::class_<IINField>(m, "IINField", "DNP3 two-byte IIN field")
        .def(py::init<>())
        .def(py::init<uint8_t, uint8_t>(), py::arg("LSB"), py::arg("MSB"))
        .def(py::init<IINBit>(), py::arg("bit"), "Construct with a single IIN bit set")
        .def_static("Empty", &IINField::Empty, "Create an empty IIN field with no bits set")
        .def_readwrite("LSB", &IINField::LSB)
        .def_readwrite("MSB", &IINField::MSB)
        .def("IsSet", &IINField::IsSet, py::arg("bit"), "Check if a specific IIN bit is set")
        .def("IsClear", &IINField::IsClear, py::arg("bit"), "Check if a specific IIN bit is clear")
        .def("SetBit", &IINField::SetBit, py::arg("bit"), "Set a specific IIN bit")
        .def("ClearBit", &IINField::ClearBit, py::arg("bit"), "Clear a specific IIN bit")
        .def("SetBitToValue", &IINField::SetBitToValue, py::arg("bit"), py::arg("value"),
             "Set or clear a specific IIN bit based on a boolean value")
        .def("Any", &IINField::Any, "Returns True if any bit is set")
        .def("Clear", py::overload_cast<>(&IINField::Clear), "Clear all bits")
        .def("HasRequestError", &IINField::HasRequestError,
             "Returns True if any request error bit is set (FUNC_NOT_SUPPORTED, OBJECT_UNKNOWN, PARAM_ERROR)")
        .def("__eq__", &IINField::operator==, py::arg("other"))
        .def("__or__", &IINField::operator|, py::arg("other"))
        .def("__ior__", &IINField::operator|=, py::arg("other"))
        .def("__and__", &IINField::operator&, py::arg("other"))
        .def("__iand__", &IINField::operator&=, py::arg("other"))
        .def("__invert__", &IINField::operator~)
        .def("__repr__",
             [](const IINField& self) {
                 std::string bits;
                 auto check = [&](IINBit bit, const char* name) {
                     if (self.IsSet(bit))
                     {
                         if (!bits.empty())
                             bits += " | ";
                         bits += name;
                     }
                 };
                 check(IINBit::BROADCAST, "BROADCAST");
                 check(IINBit::CLASS1_EVENTS, "CLASS1_EVENTS");
                 check(IINBit::CLASS2_EVENTS, "CLASS2_EVENTS");
                 check(IINBit::CLASS3_EVENTS, "CLASS3_EVENTS");
                 check(IINBit::NEED_TIME, "NEED_TIME");
                 check(IINBit::LOCAL_CONTROL, "LOCAL_CONTROL");
                 check(IINBit::DEVICE_TROUBLE, "DEVICE_TROUBLE");
                 check(IINBit::DEVICE_RESTART, "DEVICE_RESTART");
                 check(IINBit::FUNC_NOT_SUPPORTED, "FUNC_NOT_SUPPORTED");
                 check(IINBit::OBJECT_UNKNOWN, "OBJECT_UNKNOWN");
                 check(IINBit::PARAM_ERROR, "PARAM_ERROR");
                 check(IINBit::EVENT_BUFFER_OVERFLOW, "EVENT_BUFFER_OVERFLOW");
                 check(IINBit::ALREADY_EXECUTING, "ALREADY_EXECUTING");
                 check(IINBit::CONFIG_CORRUPT, "CONFIG_CORRUPT");
                 check(IINBit::RESERVED1, "RESERVED1");
                 check(IINBit::RESERVED2, "RESERVED2");
                 if (bits.empty())
                     bits = "empty";
                 char buf[80];
                 snprintf(buf, sizeof(buf), "IINField(0x%02x, 0x%02x", self.LSB, self.MSB);
                 return std::string(buf) + ", " + bits + ")";
             })
        .def("__str__", [](const IINField& self) {
            std::string bits;
            auto check = [&](IINBit bit, const char* name) {
                if (self.IsSet(bit))
                {
                    if (!bits.empty())
                        bits += ", ";
                    bits += name;
                }
            };
            check(IINBit::BROADCAST, "BROADCAST");
            check(IINBit::CLASS1_EVENTS, "CLASS1_EVENTS");
            check(IINBit::CLASS2_EVENTS, "CLASS2_EVENTS");
            check(IINBit::CLASS3_EVENTS, "CLASS3_EVENTS");
            check(IINBit::NEED_TIME, "NEED_TIME");
            check(IINBit::LOCAL_CONTROL, "LOCAL_CONTROL");
            check(IINBit::DEVICE_TROUBLE, "DEVICE_TROUBLE");
            check(IINBit::DEVICE_RESTART, "DEVICE_RESTART");
            check(IINBit::FUNC_NOT_SUPPORTED, "FUNC_NOT_SUPPORTED");
            check(IINBit::OBJECT_UNKNOWN, "OBJECT_UNKNOWN");
            check(IINBit::PARAM_ERROR, "PARAM_ERROR");
            check(IINBit::EVENT_BUFFER_OVERFLOW, "EVENT_BUFFER_OVERFLOW");
            check(IINBit::ALREADY_EXECUTING, "ALREADY_EXECUTING");
            check(IINBit::CONFIG_CORRUPT, "CONFIG_CORRUPT");
            check(IINBit::RESERVED1, "RESERVED1");
            check(IINBit::RESERVED2, "RESERVED2");
            if (bits.empty())
                return std::string("IIN: (none)");
            return std::string("IIN: ") + bits;
        });

    // ApplicationIIN
    py::class_<ApplicationIIN>(m, "ApplicationIIN", "IIN bits controllable by the outstation application")
        .def(py::init<>())
        .def_readwrite("needTime", &ApplicationIIN::needTime)
        .def_readwrite("localControl", &ApplicationIIN::localControl)
        .def_readwrite("deviceTrouble", &ApplicationIIN::deviceTrouble)
        .def_readwrite("configCorrupt", &ApplicationIIN::configCorrupt)
        .def_readwrite("eventBufferOverflow", &ApplicationIIN::eventBufferOverflow)
        .def("ToIIN", &ApplicationIIN::ToIIN, "Convert to an IINField with the corresponding bits set");

    // TaskId
    py::class_<TaskId>(m, "TaskId", "Identifier for a master task")
        .def("GetId", &TaskId::GetId)
        .def("IsDefined", &TaskId::IsDefined)
        .def("__repr__", [](const TaskId& self) {
            if (self.IsDefined())
                return std::string("TaskId(") + std::to_string(self.GetId()) + ")";
            return std::string("TaskId(undefined)");
        });

    // TaskInfo
    py::class_<TaskInfo>(m, "TaskInfo", "Info about a completed or failed task")
        .def_readonly("type", &TaskInfo::type)
        .def_readonly("result", &TaskInfo::result)
        .def_readonly("id", &TaskInfo::id)
        .def("__repr__", [](const TaskInfo& self) {
            return std::string("TaskInfo(type=") + MasterTaskTypeSpec::to_human_string(self.type)
                + ", result=" + TaskCompletionSpec::to_human_string(self.result) + ")";
        });

    // HeaderInfo
    py::class_<HeaderInfo>(m, "HeaderInfo", "Info about a response object header")
        .def(py::init<>())
        .def_readonly("gv", &HeaderInfo::gv)
        .def_readonly("qualifier", &HeaderInfo::qualifier)
        .def_readonly("tsquality", &HeaderInfo::tsquality)
        .def_readonly("isEventVariation", &HeaderInfo::isEventVariation)
        .def_readonly("flagsValid", &HeaderInfo::flagsValid)
        .def_readonly("headerIndex", &HeaderInfo::headerIndex);

    // CommandPointResult
    py::class_<CommandPointResult>(m, "CommandPointResult", "Result of a command operation on a point")
        .def_readonly("headerIndex", &CommandPointResult::headerIndex)
        .def_readonly("index", &CommandPointResult::index)
        .def_readonly("state", &CommandPointResult::state)
        .def_readonly("status", &CommandPointResult::status);

    // RestartOperationResult
    py::class_<RestartOperationResult>(m, "RestartOperationResult", "Result of a restart operation")
        .def(py::init<>())
        .def_readonly("summary", &RestartOperationResult::summary)
        .def_readonly("restartTime", &RestartOperationResult::restartTime);

    // SerialSettings
    py::class_<SerialSettings>(m, "SerialSettings", "Serial port settings")
        .def(py::init<>())
        .def_readwrite("deviceName", &SerialSettings::deviceName)
        .def_readwrite("baud", &SerialSettings::baud)
        .def_readwrite("dataBits", &SerialSettings::dataBits)
        .def_readwrite("stopBits", &SerialSettings::stopBits)
        .def_readwrite("parity", &SerialSettings::parity)
        .def_readwrite("flowType", &SerialSettings::flowType);

    // TimeDuration
    py::class_<TimeDuration>(m, "TimeDuration", "Millisecond-based time duration")
        .def(py::init<>())
        .def_static("Milliseconds", &TimeDuration::Milliseconds, py::arg("ms"))
        .def_static("Seconds", &TimeDuration::Seconds, py::arg("seconds"))
        .def_static("Minutes", &TimeDuration::Minutes, py::arg("minutes"))
        .def_static("Zero", &TimeDuration::Zero)
        .def_static("Max", &TimeDuration::Max)
        .def("__repr__", &TimeDuration::ToString);

    // LogLevel and LogLevels
    py::class_<LogLevel>(m, "LogLevel")
        .def(py::init<>())
        .def(py::init<int32_t>(), py::arg("level"))
        .def_readwrite("value", &LogLevel::value);

    py::class_<LogLevels>(m, "LogLevels", "Bitfield of log levels")
        .def(py::init<>())
        .def(py::init<int32_t>(), py::arg("levels"))
        .def(py::init<LogLevel>(), py::arg("level"))
        .def_static("none", &LogLevels::none)
        .def_static("everything", &LogLevels::everything)
        .def("get_value", &LogLevels::get_value)
        .def("__or__", py::overload_cast<const LogLevel&>(&LogLevels::operator|, py::const_))
        .def("__or__", py::overload_cast<const LogLevels&>(&LogLevels::operator|, py::const_));

    // Expose common log level constants
    auto flags_mod = m.def_submodule("flags", "Individual log level flags");
    flags_mod.attr("EVENT") = flags::EVENT;
    flags_mod.attr("ERR") = flags::ERR;
    flags_mod.attr("WARN") = flags::WARN;
    flags_mod.attr("INFO") = flags::INFO;
    flags_mod.attr("DBG") = flags::DBG;
    flags_mod.attr("LINK_RX") = flags::LINK_RX;
    flags_mod.attr("LINK_TX") = flags::LINK_TX;
    flags_mod.attr("TRANSPORT_RX") = flags::TRANSPORT_RX;
    flags_mod.attr("TRANSPORT_TX") = flags::TRANSPORT_TX;
    flags_mod.attr("APP_HEADER_RX") = flags::APP_HEADER_RX;
    flags_mod.attr("APP_HEADER_TX") = flags::APP_HEADER_TX;
    flags_mod.attr("APP_OBJECT_RX") = flags::APP_OBJECT_RX;
    flags_mod.attr("APP_OBJECT_TX") = flags::APP_OBJECT_TX;

    auto levels_mod = m.def_submodule("levels", "Predefined log level combinations");
    levels_mod.attr("NOTHING") = levels::NOTHING;
    levels_mod.attr("ALL") = levels::ALL;
    levels_mod.attr("NORMAL") = levels::NORMAL;
    levels_mod.attr("ALL_APP_COMMS") = levels::ALL_APP_COMMS;
    levels_mod.attr("ALL_COMMS") = levels::ALL_COMMS;

    // DNPTime
    py::class_<DNPTime>(m, "DNPTime", "DNP3 timestamp")
        .def(py::init<>())
        .def(py::init<uint64_t>(), py::arg("value"))
        .def(py::init<uint64_t, TimestampQuality>(), py::arg("value"), py::arg("quality"))
        .def_readwrite("value", &DNPTime::value)
        .def_readwrite("quality", &DNPTime::quality);

    // Flags
    py::class_<Flags>(m, "Flags", "Measurement quality flags")
        .def(py::init<>())
        .def(py::init<uint8_t>(), py::arg("value"))
        .def_readwrite("value", &Flags::value);

    // Measurement types
    py::class_<Binary>(m, "Binary", "Binary input measurement (on/off)")
        .def(py::init<>())
        .def(py::init<bool>(), py::arg("value"))
        .def(py::init<bool, Flags>(), py::arg("value"), py::arg("flags"))
        .def(py::init<bool, Flags, DNPTime>(), py::arg("value"), py::arg("flags"), py::arg("time"))
        .def_readwrite("value", &Binary::value)
        .def_readwrite("flags", &Binary::flags)
        .def_readwrite("time", &Binary::time);

    py::class_<DoubleBitBinary>(m, "DoubleBitBinary", "Double-bit binary input")
        .def(py::init<>())
        .def(py::init<DoubleBit>(), py::arg("value"))
        .def_readwrite("value", &DoubleBitBinary::value)
        .def_readwrite("flags", &DoubleBitBinary::flags)
        .def_readwrite("time", &DoubleBitBinary::time);

    py::class_<Analog>(m, "Analog", "Analog input measurement")
        .def(py::init<>())
        .def(py::init<double>(), py::arg("value"))
        .def(py::init<double, Flags>(), py::arg("value"), py::arg("flags"))
        .def(py::init<double, Flags, DNPTime>(), py::arg("value"), py::arg("flags"), py::arg("time"))
        .def_readwrite("value", &Analog::value)
        .def_readwrite("flags", &Analog::flags)
        .def_readwrite("time", &Analog::time);

    py::class_<Counter>(m, "Counter", "Counter measurement")
        .def(py::init<>())
        .def(py::init<uint32_t>(), py::arg("value"))
        .def(py::init<uint32_t, Flags>(), py::arg("value"), py::arg("flags"))
        .def(py::init<uint32_t, Flags, DNPTime>(), py::arg("value"), py::arg("flags"), py::arg("time"))
        .def_readwrite("value", &Counter::value)
        .def_readwrite("flags", &Counter::flags)
        .def_readwrite("time", &Counter::time);

    py::class_<FrozenCounter>(m, "FrozenCounter", "Frozen counter measurement")
        .def(py::init<>())
        .def(py::init<uint32_t>(), py::arg("value"))
        .def(py::init<uint32_t, Flags>(), py::arg("value"), py::arg("flags"))
        .def(py::init<uint32_t, Flags, DNPTime>(), py::arg("value"), py::arg("flags"), py::arg("time"))
        .def_readwrite("value", &FrozenCounter::value)
        .def_readwrite("flags", &FrozenCounter::flags)
        .def_readwrite("time", &FrozenCounter::time);

    py::class_<BinaryOutputStatus>(m, "BinaryOutputStatus", "Binary output status")
        .def(py::init<>())
        .def(py::init<bool>(), py::arg("value"))
        .def_readwrite("value", &BinaryOutputStatus::value)
        .def_readwrite("flags", &BinaryOutputStatus::flags)
        .def_readwrite("time", &BinaryOutputStatus::time);

    py::class_<AnalogOutputStatus>(m, "AnalogOutputStatus", "Analog output status")
        .def(py::init<>())
        .def(py::init<double>(), py::arg("value"))
        .def_readwrite("value", &AnalogOutputStatus::value)
        .def_readwrite("flags", &AnalogOutputStatus::flags)
        .def_readwrite("time", &AnalogOutputStatus::time);

    py::class_<TimeAndInterval>(m, "TimeAndInterval", "Time and interval measurement")
        .def(py::init<>())
        .def(py::init<DNPTime, uint32_t, IntervalUnits>(), py::arg("time"), py::arg("interval"), py::arg("units"))
        .def_readwrite("time", &TimeAndInterval::time)
        .def_readwrite("interval", &TimeAndInterval::interval)
        .def_readwrite("units", &TimeAndInterval::units);

    // BinaryCommandEvent
    py::class_<BinaryCommandEvent>(m, "BinaryCommandEvent", "Binary command event (Group13)")
        .def(py::init<>())
        .def(py::init<bool, CommandStatus>(), py::arg("value"), py::arg("status"))
        .def(py::init<bool, CommandStatus, DNPTime>(), py::arg("value"), py::arg("status"), py::arg("time"))
        .def_readwrite("value", &BinaryCommandEvent::value)
        .def_readwrite("status", &BinaryCommandEvent::status)
        .def_readwrite("time", &BinaryCommandEvent::time);

    // AnalogCommandEvent
    py::class_<AnalogCommandEvent>(m, "AnalogCommandEvent", "Analog command event (Group43)")
        .def(py::init<>())
        .def(py::init<double, CommandStatus>(), py::arg("value"), py::arg("status"))
        .def(py::init<double, CommandStatus, DNPTime>(), py::arg("value"), py::arg("status"), py::arg("time"))
        .def_readwrite("value", &AnalogCommandEvent::value)
        .def_readwrite("status", &AnalogCommandEvent::status)
        .def_readwrite("time", &AnalogCommandEvent::time);

    // AnalogInputDeadband
    py::class_<AnalogInputDeadband>(m, "AnalogInputDeadband", "Analog input deadband (Group34)")
        .def(py::init<>())
        .def(py::init<double>(), py::arg("value"))
        .def_readwrite("value", &AnalogInputDeadband::value);

    // OctetString
    py::class_<OctetString>(m, "OctetString", "Octet string data (Group110/111)")
        .def(py::init<>())
        .def(py::init<const char*>(), py::arg("input"))
        .def("Size", &OctetString::Size)
        .def("ToBytes", [](const OctetString& self) {
            auto buf = self.ToBuffer();
            return py::bytes(reinterpret_cast<const char*>(buf.data), buf.length);
        });

    // Indexed<T> types for all measurement types
    py::class_<Indexed<Binary>>(m, "IndexedBinary", "Indexed Binary measurement")
        .def(py::init<>())
        .def(py::init<const Binary&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<Binary>::value)
        .def_readwrite("index", &Indexed<Binary>::index);

    py::class_<Indexed<DoubleBitBinary>>(m, "IndexedDoubleBitBinary", "Indexed DoubleBitBinary measurement")
        .def(py::init<>())
        .def(py::init<const DoubleBitBinary&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<DoubleBitBinary>::value)
        .def_readwrite("index", &Indexed<DoubleBitBinary>::index);

    py::class_<Indexed<Analog>>(m, "IndexedAnalog", "Indexed Analog measurement")
        .def(py::init<>())
        .def(py::init<const Analog&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<Analog>::value)
        .def_readwrite("index", &Indexed<Analog>::index);

    py::class_<Indexed<Counter>>(m, "IndexedCounter", "Indexed Counter measurement")
        .def(py::init<>())
        .def(py::init<const Counter&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<Counter>::value)
        .def_readwrite("index", &Indexed<Counter>::index);

    py::class_<Indexed<FrozenCounter>>(m, "IndexedFrozenCounter", "Indexed FrozenCounter measurement")
        .def(py::init<>())
        .def(py::init<const FrozenCounter&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<FrozenCounter>::value)
        .def_readwrite("index", &Indexed<FrozenCounter>::index);

    py::class_<Indexed<BinaryOutputStatus>>(m, "IndexedBinaryOutputStatus", "Indexed BinaryOutputStatus measurement")
        .def(py::init<>())
        .def(py::init<const BinaryOutputStatus&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<BinaryOutputStatus>::value)
        .def_readwrite("index", &Indexed<BinaryOutputStatus>::index);

    py::class_<Indexed<AnalogOutputStatus>>(m, "IndexedAnalogOutputStatus", "Indexed AnalogOutputStatus measurement")
        .def(py::init<>())
        .def(py::init<const AnalogOutputStatus&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<AnalogOutputStatus>::value)
        .def_readwrite("index", &Indexed<AnalogOutputStatus>::index);

    py::class_<Indexed<OctetString>>(m, "IndexedOctetString", "Indexed OctetString measurement")
        .def(py::init<>())
        .def(py::init<const OctetString&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<OctetString>::value)
        .def_readwrite("index", &Indexed<OctetString>::index);

    py::class_<Indexed<TimeAndInterval>>(m, "IndexedTimeAndInterval", "Indexed TimeAndInterval measurement")
        .def(py::init<>())
        .def(py::init<const TimeAndInterval&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<TimeAndInterval>::value)
        .def_readwrite("index", &Indexed<TimeAndInterval>::index);

    py::class_<Indexed<BinaryCommandEvent>>(m, "IndexedBinaryCommandEvent", "Indexed BinaryCommandEvent")
        .def(py::init<>())
        .def(py::init<const BinaryCommandEvent&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<BinaryCommandEvent>::value)
        .def_readwrite("index", &Indexed<BinaryCommandEvent>::index);

    py::class_<Indexed<AnalogCommandEvent>>(m, "IndexedAnalogCommandEvent", "Indexed AnalogCommandEvent")
        .def(py::init<>())
        .def(py::init<const AnalogCommandEvent&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<AnalogCommandEvent>::value)
        .def_readwrite("index", &Indexed<AnalogCommandEvent>::index);

    py::class_<Indexed<AnalogInputDeadband>>(m, "IndexedAnalogInputDeadband", "Indexed AnalogInputDeadband")
        .def(py::init<>())
        .def(py::init<const AnalogInputDeadband&, uint16_t>(), py::arg("value"), py::arg("index"))
        .def_readwrite("value", &Indexed<AnalogInputDeadband>::value)
        .def_readwrite("index", &Indexed<AnalogInputDeadband>::index);

    // Command types
    py::class_<ControlRelayOutputBlock>(m, "ControlRelayOutputBlock", "CROB command")
        .def(py::init<OperationType, TripCloseCode, bool, uint8_t, uint32_t, uint32_t, CommandStatus>(),
             py::arg("opType") = OperationType::LATCH_ON, py::arg("tcc") = TripCloseCode::NUL, py::arg("clear") = false,
             py::arg("count") = 1, py::arg("onTime") = 100, py::arg("offTime") = 100,
             py::arg("status") = CommandStatus::SUCCESS)
        .def_readwrite("opType", &ControlRelayOutputBlock::opType)
        .def_readwrite("tcc", &ControlRelayOutputBlock::tcc)
        .def_readwrite("clear", &ControlRelayOutputBlock::clear)
        .def_readwrite("count", &ControlRelayOutputBlock::count)
        .def_readwrite("onTimeMS", &ControlRelayOutputBlock::onTimeMS)
        .def_readwrite("offTimeMS", &ControlRelayOutputBlock::offTimeMS)
        .def_readwrite("status", &ControlRelayOutputBlock::status);

    py::class_<AnalogOutputInt16>(m, "AnalogOutputInt16", "16-bit integer analog output")
        .def(py::init<>())
        .def(py::init<int16_t>(), py::arg("value"))
        .def(py::init<int16_t, CommandStatus>(), py::arg("value"), py::arg("status"))
        .def_readwrite("value", &AnalogOutputInt16::value)
        .def_readwrite("status", &AnalogOutputInt16::status);

    py::class_<AnalogOutputInt32>(m, "AnalogOutputInt32", "32-bit integer analog output")
        .def(py::init<>())
        .def(py::init<int32_t>(), py::arg("value"))
        .def(py::init<int32_t, CommandStatus>(), py::arg("value"), py::arg("status"))
        .def_readwrite("value", &AnalogOutputInt32::value)
        .def_readwrite("status", &AnalogOutputInt32::status);

    py::class_<AnalogOutputFloat32>(m, "AnalogOutputFloat32", "32-bit float analog output")
        .def(py::init<>())
        .def(py::init<float>(), py::arg("value"))
        .def(py::init<float, CommandStatus>(), py::arg("value"), py::arg("status"))
        .def_readwrite("value", &AnalogOutputFloat32::value)
        .def_readwrite("status", &AnalogOutputFloat32::status);

    py::class_<AnalogOutputDouble64>(m, "AnalogOutputDouble64", "64-bit float analog output")
        .def(py::init<>())
        .def(py::init<double>(), py::arg("value"))
        .def(py::init<double, CommandStatus>(), py::arg("value"), py::arg("status"))
        .def_readwrite("value", &AnalogOutputDouble64::value)
        .def_readwrite("status", &AnalogOutputDouble64::status);

    // IPEndpoint
    py::class_<IPEndpoint>(m, "IPEndpoint", "IP address and port")
        .def(py::init<const std::string&, uint16_t>(), py::arg("address"), py::arg("port"))
        .def_static("AllAdapters", &IPEndpoint::AllAdapters, py::arg("port"))
        .def_static("Localhost", &IPEndpoint::Localhost, py::arg("port"))
        .def_readwrite("address", &IPEndpoint::address)
        .def_readwrite("port", &IPEndpoint::port);

    // ChannelRetry
    py::class_<ChannelRetry>(m, "ChannelRetry", "Channel reconnect retry parameters")
        .def(py::init<TimeDuration, TimeDuration>(), py::arg("minOpenRetry"), py::arg("maxOpenRetry"))
        .def_static("Default", &ChannelRetry::Default)
        .def_readwrite("minOpenRetry", &ChannelRetry::minOpenRetry)
        .def_readwrite("maxOpenRetry", &ChannelRetry::maxOpenRetry)
        .def_readwrite("reconnectDelay", &ChannelRetry::reconnectDelay);

    // LinkConfig
    py::class_<LinkConfig>(m, "LinkConfig", "Link layer configuration")
        .def(py::init<bool>(), py::arg("isMaster"))
        .def(py::init<bool, uint16_t, uint16_t, TimeDuration, TimeDuration>(), py::arg("isMaster"),
             py::arg("localAddr"), py::arg("remoteAddr"), py::arg("timeout"), py::arg("keepAliveTimeout"))
        .def_readwrite("IsMaster", &LinkConfig::IsMaster)
        .def_readwrite("LocalAddr", &LinkConfig::LocalAddr)
        .def_readwrite("RemoteAddr", &LinkConfig::RemoteAddr)
        .def_readwrite("Timeout", &LinkConfig::Timeout)
        .def_readwrite("KeepAliveTimeout", &LinkConfig::KeepAliveTimeout);

    // MasterParams
    py::class_<MasterParams>(m, "MasterParams", "Master configuration parameters")
        .def(py::init<>())
        .def_readwrite("responseTimeout", &MasterParams::responseTimeout)
        .def_readwrite("timeSyncMode", &MasterParams::timeSyncMode)
        .def_readwrite("disableUnsolOnStartup", &MasterParams::disableUnsolOnStartup)
        .def_readwrite("ignoreRestartIIN", &MasterParams::ignoreRestartIIN)
        .def_readwrite("integrityOnEventOverflowIIN", &MasterParams::integrityOnEventOverflowIIN)
        .def_readwrite("taskRetryPeriod", &MasterParams::taskRetryPeriod)
        .def_readwrite("maxTaskRetryPeriod", &MasterParams::maxTaskRetryPeriod)
        .def_readwrite("taskStartTimeout", &MasterParams::taskStartTimeout)
        .def_readwrite("maxTxFragSize", &MasterParams::maxTxFragSize)
        .def_readwrite("maxRxFragSize", &MasterParams::maxRxFragSize)
        .def_readwrite("controlQualifierMode", &MasterParams::controlQualifierMode);

    // MasterStackConfig
    py::class_<MasterStackConfig>(m, "MasterStackConfig", "Complete master stack configuration")
        .def(py::init<>())
        .def_readwrite("master", &MasterStackConfig::master)
        .def_readwrite("link", &MasterStackConfig::link);

    // EventBufferConfig
    py::class_<EventBufferConfig>(m, "EventBufferConfig", "Outstation event buffer sizes")
        .def(py::init<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>(),
             py::arg("maxBinaryEvents") = 0, py::arg("maxDoubleBinaryEvents") = 0, py::arg("maxAnalogEvents") = 0,
             py::arg("maxCounterEvents") = 0, py::arg("maxFrozenCounterEvents") = 0,
             py::arg("maxBinaryOutputStatusEvents") = 0, py::arg("maxAnalogOutputStatusEvents") = 0,
             py::arg("maxOctetStringEvents") = 0)
        .def_static("AllTypes", &EventBufferConfig::AllTypes, py::arg("sizes"))
        .def_readwrite("maxBinaryEvents", &EventBufferConfig::maxBinaryEvents)
        .def_readwrite("maxDoubleBinaryEvents", &EventBufferConfig::maxDoubleBinaryEvents)
        .def_readwrite("maxAnalogEvents", &EventBufferConfig::maxAnalogEvents)
        .def_readwrite("maxCounterEvents", &EventBufferConfig::maxCounterEvents)
        .def_readwrite("maxFrozenCounterEvents", &EventBufferConfig::maxFrozenCounterEvents)
        .def_readwrite("maxBinaryOutputStatusEvents", &EventBufferConfig::maxBinaryOutputStatusEvents)
        .def_readwrite("maxAnalogOutputStatusEvents", &EventBufferConfig::maxAnalogOutputStatusEvents)
        .def_readwrite("maxOctetStringEvents", &EventBufferConfig::maxOctetStringEvents);

    // OutstationParams
    py::class_<OutstationParams>(m, "OutstationParams", "Outstation configuration parameters")
        .def(py::init<>())
        .def_readwrite("maxControlsPerRequest", &OutstationParams::maxControlsPerRequest)
        .def_readwrite("selectTimeout", &OutstationParams::selectTimeout)
        .def_readwrite("solConfirmTimeout", &OutstationParams::solConfirmTimeout)
        .def_readwrite("unsolConfirmTimeout", &OutstationParams::unsolConfirmTimeout)
        .def_readwrite("maxTxFragSize", &OutstationParams::maxTxFragSize)
        .def_readwrite("maxRxFragSize", &OutstationParams::maxRxFragSize)
        .def_readwrite("allowUnsolicited", &OutstationParams::allowUnsolicited)
        .def_readwrite("respondToAnyMaster", &OutstationParams::respondToAnyMaster);

    // OutstationConfig
    py::class_<OutstationConfig>(m, "OutstationConfig", "Outstation config with params and event buffer")
        .def(py::init<>())
        .def_readwrite("params", &OutstationConfig::params)
        .def_readwrite("eventBufferConfig", &OutstationConfig::eventBufferConfig);

    // DatabaseConfig
    py::class_<DatabaseConfig>(m, "DatabaseConfig", "Outstation database point configuration")
        .def(py::init<>())
        .def(py::init<uint16_t>(), py::arg("all_types"), "Create config with the same number of points for all types")
        .def_readwrite("binary_input", &DatabaseConfig::binary_input)
        .def_readwrite("double_binary", &DatabaseConfig::double_binary)
        .def_readwrite("analog_input", &DatabaseConfig::analog_input)
        .def_readwrite("counter", &DatabaseConfig::counter)
        .def_readwrite("frozen_counter", &DatabaseConfig::frozen_counter)
        .def_readwrite("binary_output_status", &DatabaseConfig::binary_output_status)
        .def_readwrite("analog_output_status", &DatabaseConfig::analog_output_status)
        .def_readwrite("time_and_interval", &DatabaseConfig::time_and_interval)
        .def_readwrite("octet_string", &DatabaseConfig::octet_string);

    // OutstationStackConfig
    py::class_<OutstationStackConfig>(m, "OutstationStackConfig", "Complete outstation stack configuration")
        .def(py::init<>())
        .def(py::init<const DatabaseConfig&>(), py::arg("database"))
        .def_readwrite("database", &OutstationStackConfig::database)
        .def_readwrite("outstation", &OutstationStackConfig::outstation)
        .def_readwrite("link", &OutstationStackConfig::link);

    // UpdateBuilder + Updates
    py::class_<Updates>(m, "Updates", "A set of measurement updates to apply to an outstation")
        .def("IsEmpty", &Updates::IsEmpty);

    py::class_<UpdateBuilder>(m, "UpdateBuilder", "Builder for creating outstation measurement updates")
        .def(py::init<>())
        .def("Update", py::overload_cast<const Binary&, uint16_t, EventMode>(&UpdateBuilder::Update), py::arg("meas"),
             py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const DoubleBitBinary&, uint16_t, EventMode>(&UpdateBuilder::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const Analog&, uint16_t, EventMode>(&UpdateBuilder::Update), py::arg("meas"),
             py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const Counter&, uint16_t, EventMode>(&UpdateBuilder::Update), py::arg("meas"),
             py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const BinaryOutputStatus&, uint16_t, EventMode>(&UpdateBuilder::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const AnalogOutputStatus&, uint16_t, EventMode>(&UpdateBuilder::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const TimeAndInterval&, uint16_t>(&UpdateBuilder::Update), py::arg("meas"),
             py::arg("index"))
        .def("FreezeCounter", &UpdateBuilder::FreezeCounter, py::arg("index"), py::arg("clear"),
             py::arg("mode") = EventMode::Detect)
        .def("Modify", &UpdateBuilder::Modify, py::arg("type"), py::arg("start"), py::arg("stop"), py::arg("flags"))
        .def("Build", &UpdateBuilder::Build);

    // IUpdateHandler
    py::class_<IUpdateHandler>(m, "IUpdateHandler", "Interface for updating outstation measurement values")
        .def("Update", py::overload_cast<const Binary&, uint16_t, EventMode>(&IUpdateHandler::Update), py::arg("meas"),
             py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const DoubleBitBinary&, uint16_t, EventMode>(&IUpdateHandler::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const Analog&, uint16_t, EventMode>(&IUpdateHandler::Update), py::arg("meas"),
             py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const Counter&, uint16_t, EventMode>(&IUpdateHandler::Update), py::arg("meas"),
             py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("FreezeCounter", &IUpdateHandler::FreezeCounter, py::arg("index"), py::arg("clear") = false,
             py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const BinaryOutputStatus&, uint16_t, EventMode>(&IUpdateHandler::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const AnalogOutputStatus&, uint16_t, EventMode>(&IUpdateHandler::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const OctetString&, uint16_t, EventMode>(&IUpdateHandler::Update),
             py::arg("meas"), py::arg("index"), py::arg("mode") = EventMode::Detect)
        .def("Update", py::overload_cast<const TimeAndInterval&, uint16_t>(&IUpdateHandler::Update), py::arg("meas"),
             py::arg("index"))
        .def("Modify", &IUpdateHandler::Modify, py::arg("type"), py::arg("start"), py::arg("stop"), py::arg("flags"));

    // ===== File Transfer Types =====

    // FilePermissionSet
    py::class_<FilePermissionSet>(m, "FilePermissionSet", "UNIX-style permission set for a single class")
        .def(py::init<>())
        .def(py::init<bool, bool, bool>(), py::arg("read"), py::arg("write"), py::arg("execute"))
        .def_readwrite("read", &FilePermissionSet::read)
        .def_readwrite("write", &FilePermissionSet::write)
        .def_readwrite("execute", &FilePermissionSet::execute);

    // FilePermissions
    py::class_<FilePermissions>(m, "FilePermissions", "DNP3 file permissions (16-bit UNIX-style)")
        .def(py::init<>())
        .def(py::init<FilePermissionSet, FilePermissionSet, FilePermissionSet>(), py::arg("owner"), py::arg("group"),
             py::arg("world"))
        .def_readwrite("owner", &FilePermissions::owner)
        .def_readwrite("group", &FilePermissions::group)
        .def_readwrite("world", &FilePermissions::world)
        .def_static("FromRaw", &FilePermissions::FromRaw, py::arg("raw"))
        .def("ToRaw", &FilePermissions::ToRaw);

    // FileInfo
    py::class_<FileInfo>(m, "FileInfo", "Information about a file from GET_FILE_INFO")
        .def(py::init<>())
        .def_readwrite("fileName", &FileInfo::fileName)
        .def_readwrite("type", &FileInfo::type)
        .def_readwrite("size", &FileInfo::size)
        .def_readwrite("timeOfCreation", &FileInfo::timeOfCreation)
        .def_readwrite("permissions", &FileInfo::permissions)
        .def_readwrite("requestId", &FileInfo::requestId);

    // FileReadResult
    py::class_<FileReadResult>(m, "FileReadResult", "Result of a file read operation")
        .def(py::init<>())
        .def_readonly("summary", &FileReadResult::summary)
        .def_readonly("statusCode", &FileReadResult::statusCode)
        .def_property_readonly("data", [](const FileReadResult& self) {
            return py::bytes(reinterpret_cast<const char*>(self.data.data()), self.data.size());
        });

    // FileInfoResult
    py::class_<FileInfoResult>(m, "FileInfoResult", "Result of a get-file-info operation")
        .def(py::init<>())
        .def_readonly("summary", &FileInfoResult::summary)
        .def_readonly("statusCode", &FileInfoResult::statusCode)
        .def_readonly("info", &FileInfoResult::info);

    // FileOperationResult
    py::class_<FileOperationResult>(m, "FileOperationResult", "Result of a simple file operation")
        .def(py::init<>())
        .def_readonly("summary", &FileOperationResult::summary)
        .def_readonly("statusCode", &FileOperationResult::statusCode);

    // FileWriteResult
    py::class_<FileWriteResult>(m, "FileWriteResult", "Result of a file write operation")
        .def(py::init<>())
        .def_readonly("summary", &FileWriteResult::summary)
        .def_readonly("statusCode", &FileWriteResult::statusCode);

    // DirectoryReadResult
    py::class_<DirectoryReadResult>(m, "DirectoryReadResult", "Result of a directory read operation")
        .def(py::init<>())
        .def_readonly("summary", &DirectoryReadResult::summary)
        .def_readonly("statusCode", &DirectoryReadResult::statusCode)
        .def_readonly("entries", &DirectoryReadResult::entries);

    // FileAuthResult (master-side)
    py::class_<FileAuthResult_t>(m, "FileAuthResult", "Result of a file authentication operation")
        .def(py::init<>())
        .def_readonly("summary", &FileAuthResult_t::summary)
        .def_readonly("statusCode", &FileAuthResult_t::statusCode)
        .def_readonly("authKey", &FileAuthResult_t::authKey);

    // Outstation-side file result types

    // FileOpenResult
    py::class_<FileOpenResult>(m, "FileOpenResult", "Result of an OPEN_FILE request")
        .def(py::init<>())
        .def_readwrite("status", &FileOpenResult::status)
        .def_readwrite("fileHandle", &FileOpenResult::fileHandle)
        .def_readwrite("fileSize", &FileOpenResult::fileSize)
        .def_readwrite("maxBlockSize", &FileOpenResult::maxBlockSize);

    // FileBlockResult
    py::class_<FileBlockResult>(m, "FileBlockResult", "Result of a READ_BLOCK request")
        .def(py::init<>())
        .def_readwrite("status", &FileBlockResult::status)
        .def_readwrite("data", &FileBlockResult::data)
        .def_readwrite("lastBlock", &FileBlockResult::lastBlock);

    // FileCommandResult
    py::class_<FileCommandResult>(m, "FileCommandResult", "Result of a GET_FILE_INFO request")
        .def(py::init<>())
        .def_readwrite("status", &FileCommandResult::status)
        .def_readwrite("info", &FileCommandResult::info);

    // FileAuthResult (outstation-side)
    py::class_<FileAuthResult>(m, "OutstationFileAuthResult", "Result of an AUTHENTICATE_FILE request")
        .def(py::init<>())
        .def_readwrite("status", &FileAuthResult::status)
        .def_readwrite("authKey", &FileAuthResult::authKey);
}
