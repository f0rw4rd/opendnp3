/*
 * Copyright 2013-2022 Step Function I/O, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Step Function I/O
 * LLC (https://stepfunc.io) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Step Function I/O LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "opendnp3/app/IINField.h"
#include "opendnp3/gen/ChannelState.h"
#include "opendnp3/gen/CommandPointState.h"
#include "opendnp3/gen/CommandStatus.h"
#include "opendnp3/gen/DoubleBit.h"
#include "opendnp3/gen/EventMode.h"
#include "opendnp3/gen/FlagsType.h"
#include "opendnp3/gen/FlowControl.h"
#include "opendnp3/gen/FreezeType.h"
#include "opendnp3/gen/FunctionCode.h"
#include "opendnp3/gen/GroupVariation.h"
#include "opendnp3/gen/IndexQualifierMode.h"
#include "opendnp3/gen/IntervalUnits.h"
#include "opendnp3/gen/LinkStatus.h"
#include "opendnp3/gen/MasterTaskType.h"
#include "opendnp3/gen/OperateType.h"
#include "opendnp3/gen/OperationType.h"
#include "opendnp3/gen/Parity.h"
#include "opendnp3/gen/PointClass.h"
#include "opendnp3/gen/QualifierCode.h"
#include "opendnp3/gen/RestartMode.h"
#include "opendnp3/gen/RestartType.h"
#include "opendnp3/gen/ServerAcceptMode.h"
#include "opendnp3/gen/StopBits.h"
#include "opendnp3/gen/TaskCompletion.h"
#include "opendnp3/gen/TimeSyncMode.h"
#include "opendnp3/gen/TimestampQuality.h"
#include "opendnp3/gen/TripCloseCode.h"
#include "opendnp3/master/FileOperationResult.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace opendnp3;

void init_enums(py::module_& m)
{
    // ChannelState
    py::enum_<ChannelState>(m, "ChannelState", "Enumeration for possible states of a channel")
        .value("CLOSED", ChannelState::CLOSED, "Offline and idle")
        .value("OPENING", ChannelState::OPENING, "Trying to open")
        .value("OPEN", ChannelState::OPEN, "Open")
        .value("SHUTDOWN", ChannelState::SHUTDOWN, "Stopped and will never do anything again");

    // CommandStatus
    py::enum_<CommandStatus>(m, "CommandStatus",
                             "Result codes received from an outstation in response to command request")
        .value("SUCCESS", CommandStatus::SUCCESS)
        .value("TIMEOUT", CommandStatus::TIMEOUT)
        .value("NO_SELECT", CommandStatus::NO_SELECT)
        .value("FORMAT_ERROR", CommandStatus::FORMAT_ERROR)
        .value("NOT_SUPPORTED", CommandStatus::NOT_SUPPORTED)
        .value("ALREADY_ACTIVE", CommandStatus::ALREADY_ACTIVE)
        .value("HARDWARE_ERROR", CommandStatus::HARDWARE_ERROR)
        .value("LOCAL", CommandStatus::LOCAL)
        .value("TOO_MANY_OPS", CommandStatus::TOO_MANY_OPS)
        .value("NOT_AUTHORIZED", CommandStatus::NOT_AUTHORIZED)
        .value("AUTOMATION_INHIBIT", CommandStatus::AUTOMATION_INHIBIT)
        .value("PROCESSING_LIMITED", CommandStatus::PROCESSING_LIMITED)
        .value("OUT_OF_RANGE", CommandStatus::OUT_OF_RANGE)
        .value("DOWNSTREAM_LOCAL", CommandStatus::DOWNSTREAM_LOCAL)
        .value("ALREADY_COMPLETE", CommandStatus::ALREADY_COMPLETE)
        .value("BLOCKED", CommandStatus::BLOCKED)
        .value("CANCELLED", CommandStatus::CANCELLED)
        .value("BLOCKED_OTHER_MASTER", CommandStatus::BLOCKED_OTHER_MASTER)
        .value("DOWNSTREAM_FAIL", CommandStatus::DOWNSTREAM_FAIL)
        .value("NON_PARTICIPATING", CommandStatus::NON_PARTICIPATING)
        .value("UNDEFINED", CommandStatus::UNDEFINED);

    // PointClass
    py::enum_<PointClass>(m, "PointClass", "Class assignment for a measurement point")
        .value("Class0", PointClass::Class0)
        .value("Class1", PointClass::Class1)
        .value("Class2", PointClass::Class2)
        .value("Class3", PointClass::Class3);

    // DoubleBit
    py::enum_<DoubleBit>(m, "DoubleBit", "Enumeration for double-bit binary values")
        .value("INTERMEDIATE", DoubleBit::INTERMEDIATE)
        .value("DETERMINED_OFF", DoubleBit::DETERMINED_OFF)
        .value("DETERMINED_ON", DoubleBit::DETERMINED_ON)
        .value("INDETERMINATE", DoubleBit::INDETERMINATE);

    // TimestampQuality
    py::enum_<TimestampQuality>(m, "TimestampQuality", "Enumeration for timestamp quality")
        .value("SYNCHRONIZED", TimestampQuality::SYNCHRONIZED)
        .value("UNSYNCHRONIZED", TimestampQuality::UNSYNCHRONIZED)
        .value("INVALID", TimestampQuality::INVALID);

    // FreezeType
    py::enum_<FreezeType>(m, "FreezeType", "Freeze operation types for master freeze requests")
        .value("ImmediateFreeze", FreezeType::ImmediateFreeze, "Copy current values to freeze buffer")
        .value("ImmediateFreezeNR", FreezeType::ImmediateFreezeNR, "Freeze, no response expected")
        .value("FreezeAndClear", FreezeType::FreezeAndClear, "Copy to freeze buffer, then clear")
        .value("FreezeAndClearNR", FreezeType::FreezeAndClearNR, "Freeze and clear, no response expected");

    // FunctionCode
    py::enum_<FunctionCode>(m, "FunctionCode", "DNP3 function codes")
        .value("CONFIRM", FunctionCode::CONFIRM)
        .value("READ", FunctionCode::READ)
        .value("WRITE", FunctionCode::WRITE)
        .value("SELECT", FunctionCode::SELECT)
        .value("OPERATE", FunctionCode::OPERATE)
        .value("DIRECT_OPERATE", FunctionCode::DIRECT_OPERATE)
        .value("DIRECT_OPERATE_NR", FunctionCode::DIRECT_OPERATE_NR)
        .value("IMMED_FREEZE", FunctionCode::IMMED_FREEZE)
        .value("IMMED_FREEZE_NR", FunctionCode::IMMED_FREEZE_NR)
        .value("FREEZE_CLEAR", FunctionCode::FREEZE_CLEAR)
        .value("FREEZE_CLEAR_NR", FunctionCode::FREEZE_CLEAR_NR)
        .value("FREEZE_AT_TIME", FunctionCode::FREEZE_AT_TIME)
        .value("FREEZE_AT_TIME_NR", FunctionCode::FREEZE_AT_TIME_NR)
        .value("COLD_RESTART", FunctionCode::COLD_RESTART)
        .value("WARM_RESTART", FunctionCode::WARM_RESTART)
        .value("INITIALIZE_DATA", FunctionCode::INITIALIZE_DATA)
        .value("INITIALIZE_APPLICATION", FunctionCode::INITIALIZE_APPLICATION)
        .value("START_APPLICATION", FunctionCode::START_APPLICATION)
        .value("STOP_APPLICATION", FunctionCode::STOP_APPLICATION)
        .value("ENABLE_UNSOLICITED", FunctionCode::ENABLE_UNSOLICITED)
        .value("DISABLE_UNSOLICITED", FunctionCode::DISABLE_UNSOLICITED)
        .value("ASSIGN_CLASS", FunctionCode::ASSIGN_CLASS)
        .value("DELAY_MEASURE", FunctionCode::DELAY_MEASURE)
        .value("RECORD_CURRENT_TIME", FunctionCode::RECORD_CURRENT_TIME)
        .value("OPEN_FILE", FunctionCode::OPEN_FILE)
        .value("CLOSE_FILE", FunctionCode::CLOSE_FILE)
        .value("DELETE_FILE", FunctionCode::DELETE_FILE)
        .value("GET_FILE_INFO", FunctionCode::GET_FILE_INFO)
        .value("AUTHENTICATE_FILE", FunctionCode::AUTHENTICATE_FILE)
        .value("ABORT_FILE", FunctionCode::ABORT_FILE)
        .value("ACTIVATE_CONFIG", FunctionCode::ACTIVATE_CONFIG)
        .value("AUTH_REQUEST", FunctionCode::AUTH_REQUEST)
        .value("AUTH_ERROR", FunctionCode::AUTH_ERROR)
        .value("RESPONSE", FunctionCode::RESPONSE)
        .value("UNSOLICITED_RESPONSE", FunctionCode::UNSOLICITED_RESPONSE)
        .value("AUTH_RESPONSE", FunctionCode::AUTH_RESPONSE)
        .value("UNKNOWN", FunctionCode::UNKNOWN);

    // LinkStatus
    py::enum_<LinkStatus>(m, "LinkStatus", "Enumeration for link layer status")
        .value("UNRESET", LinkStatus::UNRESET)
        .value("RESET", LinkStatus::RESET);

    // OperateType
    py::enum_<OperateType>(m, "OperateType", "Enumeration for operate type received by outstation")
        .value("SelectBeforeOperate", OperateType::SelectBeforeOperate)
        .value("DirectOperate", OperateType::DirectOperate)
        .value("DirectOperateNoAck", OperateType::DirectOperateNoAck);

    // OperationType
    py::enum_<OperationType>(m, "OperationType", "Enumeration for CROB operation type")
        .value("NUL", OperationType::NUL)
        .value("PULSE_ON", OperationType::PULSE_ON)
        .value("PULSE_OFF", OperationType::PULSE_OFF)
        .value("LATCH_ON", OperationType::LATCH_ON)
        .value("LATCH_OFF", OperationType::LATCH_OFF)
        .value("Undefined", OperationType::Undefined);

    // TripCloseCode
    py::enum_<TripCloseCode>(m, "TripCloseCode", "Trip-close code for CROB")
        .value("NUL", TripCloseCode::NUL)
        .value("CLOSE", TripCloseCode::CLOSE)
        .value("TRIP", TripCloseCode::TRIP)
        .value("RESERVED", TripCloseCode::RESERVED);

    // ServerAcceptMode
    py::enum_<ServerAcceptMode>(m, "ServerAcceptMode", "How TCP/TLS servers handle new connections")
        .value("CloseNew", ServerAcceptMode::CloseNew)
        .value("CloseExisting", ServerAcceptMode::CloseExisting);

    // TimeSyncMode
    py::enum_<TimeSyncMode>(m, "TimeSyncMode", "Enumeration for time sync mode")
        .value("None", TimeSyncMode::None)
        .value("NonLAN", TimeSyncMode::NonLAN)
        .value("LAN", TimeSyncMode::LAN);

    // IndexQualifierMode
    py::enum_<IndexQualifierMode>(m, "IndexQualifierMode", "Control how the master chooses qualifier for requests")
        .value("allow_one_byte", IndexQualifierMode::allow_one_byte)
        .value("always_two_bytes", IndexQualifierMode::always_two_bytes);

    // RestartType
    py::enum_<RestartType>(m, "RestartType", "Restart type for master restart requests")
        .value("COLD", RestartType::COLD)
        .value("WARM", RestartType::WARM);

    // RestartMode
    py::enum_<RestartMode>(m, "RestartMode", "Enumeration for outstation restart mode")
        .value("UNSUPPORTED", RestartMode::UNSUPPORTED)
        .value("SUPPORTED_DELAY_FINE", RestartMode::SUPPORTED_DELAY_FINE)
        .value("SUPPORTED_DELAY_COARSE", RestartMode::SUPPORTED_DELAY_COARSE);

    // EventMode
    py::enum_<EventMode>(m, "EventMode", "Describes how event generation is handled")
        .value("Detect", EventMode::Detect)
        .value("Force", EventMode::Force)
        .value("Suppress", EventMode::Suppress);

    // FlagsType
    py::enum_<FlagsType>(m, "FlagsType", "Enumeration describing measurement types for flag modifications")
        .value("DoubleBinaryInput", FlagsType::DoubleBinaryInput)
        .value("Counter", FlagsType::Counter)
        .value("FrozenCounter", FlagsType::FrozenCounter)
        .value("AnalogInput", FlagsType::AnalogInput)
        .value("BinaryOutputStatus", FlagsType::BinaryOutputStatus)
        .value("AnalogOutputStatus", FlagsType::AnalogOutputStatus)
        .value("BinaryInput", FlagsType::BinaryInput);

    // IntervalUnits
    py::enum_<IntervalUnits>(m, "IntervalUnits", "Time interval units")
        .value("NoRepeat", IntervalUnits::NoRepeat)
        .value("Milliseconds", IntervalUnits::Milliseconds)
        .value("Seconds", IntervalUnits::Seconds)
        .value("Minutes", IntervalUnits::Minutes)
        .value("Hours", IntervalUnits::Hours)
        .value("Days", IntervalUnits::Days)
        .value("Weeks", IntervalUnits::Weeks)
        .value("Months7", IntervalUnits::Months7)
        .value("Months8", IntervalUnits::Months8)
        .value("Months9", IntervalUnits::Months9)
        .value("Seasons", IntervalUnits::Seasons)
        .value("Undefined", IntervalUnits::Undefined);

    // MasterTaskType
    py::enum_<MasterTaskType>(m, "MasterTaskType", "Enumeration of internal master tasks")
        .value("CLEAR_RESTART", MasterTaskType::CLEAR_RESTART)
        .value("DISABLE_UNSOLICITED", MasterTaskType::DISABLE_UNSOLICITED)
        .value("ASSIGN_CLASS", MasterTaskType::ASSIGN_CLASS)
        .value("STARTUP_INTEGRITY_POLL", MasterTaskType::STARTUP_INTEGRITY_POLL)
        .value("NON_LAN_TIME_SYNC", MasterTaskType::NON_LAN_TIME_SYNC)
        .value("LAN_TIME_SYNC", MasterTaskType::LAN_TIME_SYNC)
        .value("ENABLE_UNSOLICITED", MasterTaskType::ENABLE_UNSOLICITED)
        .value("AUTO_EVENT_SCAN", MasterTaskType::AUTO_EVENT_SCAN)
        .value("USER_TASK", MasterTaskType::USER_TASK);

    // TaskCompletion
    py::enum_<TaskCompletion>(m, "TaskCompletion", "Describes if a master task succeeded or failed")
        .value("SUCCESS", TaskCompletion::SUCCESS)
        .value("FAILURE_BAD_RESPONSE", TaskCompletion::FAILURE_BAD_RESPONSE)
        .value("FAILURE_RESPONSE_TIMEOUT", TaskCompletion::FAILURE_RESPONSE_TIMEOUT)
        .value("FAILURE_START_TIMEOUT", TaskCompletion::FAILURE_START_TIMEOUT)
        .value("FAILURE_MESSAGE_FORMAT_ERROR", TaskCompletion::FAILURE_MESSAGE_FORMAT_ERROR)
        .value("FAILURE_NO_COMMS", TaskCompletion::FAILURE_NO_COMMS);

    // CommandPointState
    py::enum_<CommandPointState>(m, "CommandPointState", "State of an individual command point after an operation")
        .value("INIT", CommandPointState::INIT)
        .value("SELECT_SUCCESS", CommandPointState::SELECT_SUCCESS)
        .value("SELECT_MISMATCH", CommandPointState::SELECT_MISMATCH)
        .value("SELECT_FAIL", CommandPointState::SELECT_FAIL)
        .value("OPERATE_FAIL", CommandPointState::OPERATE_FAIL)
        .value("SUCCESS", CommandPointState::SUCCESS);

    // IINBit
    py::enum_<IINBit>(m, "IINBit", "Individual IIN bit identifiers")
        .value("BROADCAST", IINBit::BROADCAST)
        .value("CLASS1_EVENTS", IINBit::CLASS1_EVENTS)
        .value("CLASS2_EVENTS", IINBit::CLASS2_EVENTS)
        .value("CLASS3_EVENTS", IINBit::CLASS3_EVENTS)
        .value("NEED_TIME", IINBit::NEED_TIME)
        .value("LOCAL_CONTROL", IINBit::LOCAL_CONTROL)
        .value("DEVICE_TROUBLE", IINBit::DEVICE_TROUBLE)
        .value("DEVICE_RESTART", IINBit::DEVICE_RESTART)
        .value("FUNC_NOT_SUPPORTED", IINBit::FUNC_NOT_SUPPORTED)
        .value("OBJECT_UNKNOWN", IINBit::OBJECT_UNKNOWN)
        .value("PARAM_ERROR", IINBit::PARAM_ERROR)
        .value("EVENT_BUFFER_OVERFLOW", IINBit::EVENT_BUFFER_OVERFLOW)
        .value("ALREADY_EXECUTING", IINBit::ALREADY_EXECUTING)
        .value("CONFIG_CORRUPT", IINBit::CONFIG_CORRUPT)
        .value("RESERVED1", IINBit::RESERVED1)
        .value("RESERVED2", IINBit::RESERVED2);

    // StopBits
    py::enum_<StopBits>(m, "StopBits", "Serial port stop bits")
        .value("One", StopBits::One)
        .value("OnePointFive", StopBits::OnePointFive)
        .value("Two", StopBits::Two)
        .value("None", StopBits::None);

    // Parity
    py::enum_<Parity>(m, "Parity", "Serial port parity")
        .value("Even", Parity::Even)
        .value("Odd", Parity::Odd)
        .value("None", Parity::None);

    // FlowControl
    py::enum_<FlowControl>(m, "FlowControl", "Serial port flow control")
        .value("Hardware", FlowControl::Hardware)
        .value("XONXOFF", FlowControl::XONXOFF)
        .value("None", FlowControl::None);

    // GroupVariation
    py::enum_<GroupVariation>(m, "GroupVariation", "DNP3 group/variation identifiers")
        .value("Group0Var0", GroupVariation::Group0Var0)
        .value("Group0Var254", GroupVariation::Group0Var254)
        .value("Group0Var255", GroupVariation::Group0Var255)
        .value("Group1Var0", GroupVariation::Group1Var0)
        .value("Group1Var1", GroupVariation::Group1Var1)
        .value("Group1Var2", GroupVariation::Group1Var2)
        .value("Group2Var0", GroupVariation::Group2Var0)
        .value("Group2Var1", GroupVariation::Group2Var1)
        .value("Group2Var2", GroupVariation::Group2Var2)
        .value("Group2Var3", GroupVariation::Group2Var3)
        .value("Group3Var0", GroupVariation::Group3Var0)
        .value("Group3Var1", GroupVariation::Group3Var1)
        .value("Group3Var2", GroupVariation::Group3Var2)
        .value("Group4Var0", GroupVariation::Group4Var0)
        .value("Group4Var1", GroupVariation::Group4Var1)
        .value("Group4Var2", GroupVariation::Group4Var2)
        .value("Group4Var3", GroupVariation::Group4Var3)
        .value("Group10Var0", GroupVariation::Group10Var0)
        .value("Group10Var1", GroupVariation::Group10Var1)
        .value("Group10Var2", GroupVariation::Group10Var2)
        .value("Group11Var0", GroupVariation::Group11Var0)
        .value("Group11Var1", GroupVariation::Group11Var1)
        .value("Group11Var2", GroupVariation::Group11Var2)
        .value("Group12Var0", GroupVariation::Group12Var0)
        .value("Group12Var1", GroupVariation::Group12Var1)
        .value("Group12Var2", GroupVariation::Group12Var2)
        .value("Group12Var3", GroupVariation::Group12Var3)
        .value("Group13Var0", GroupVariation::Group13Var0)
        .value("Group13Var1", GroupVariation::Group13Var1)
        .value("Group13Var2", GroupVariation::Group13Var2)
        .value("Group20Var0", GroupVariation::Group20Var0)
        .value("Group20Var1", GroupVariation::Group20Var1)
        .value("Group20Var2", GroupVariation::Group20Var2)
        .value("Group20Var3", GroupVariation::Group20Var3)
        .value("Group20Var4", GroupVariation::Group20Var4)
        .value("Group20Var5", GroupVariation::Group20Var5)
        .value("Group20Var6", GroupVariation::Group20Var6)
        .value("Group20Var7", GroupVariation::Group20Var7)
        .value("Group20Var8", GroupVariation::Group20Var8)
        .value("Group21Var0", GroupVariation::Group21Var0)
        .value("Group21Var1", GroupVariation::Group21Var1)
        .value("Group21Var2", GroupVariation::Group21Var2)
        .value("Group21Var3", GroupVariation::Group21Var3)
        .value("Group21Var4", GroupVariation::Group21Var4)
        .value("Group21Var5", GroupVariation::Group21Var5)
        .value("Group21Var6", GroupVariation::Group21Var6)
        .value("Group21Var7", GroupVariation::Group21Var7)
        .value("Group21Var8", GroupVariation::Group21Var8)
        .value("Group21Var9", GroupVariation::Group21Var9)
        .value("Group21Var10", GroupVariation::Group21Var10)
        .value("Group21Var11", GroupVariation::Group21Var11)
        .value("Group21Var12", GroupVariation::Group21Var12)
        .value("Group22Var0", GroupVariation::Group22Var0)
        .value("Group22Var1", GroupVariation::Group22Var1)
        .value("Group22Var2", GroupVariation::Group22Var2)
        .value("Group22Var3", GroupVariation::Group22Var3)
        .value("Group22Var4", GroupVariation::Group22Var4)
        .value("Group22Var5", GroupVariation::Group22Var5)
        .value("Group22Var6", GroupVariation::Group22Var6)
        .value("Group22Var7", GroupVariation::Group22Var7)
        .value("Group22Var8", GroupVariation::Group22Var8)
        .value("Group23Var0", GroupVariation::Group23Var0)
        .value("Group23Var1", GroupVariation::Group23Var1)
        .value("Group23Var2", GroupVariation::Group23Var2)
        .value("Group23Var3", GroupVariation::Group23Var3)
        .value("Group23Var4", GroupVariation::Group23Var4)
        .value("Group23Var5", GroupVariation::Group23Var5)
        .value("Group23Var6", GroupVariation::Group23Var6)
        .value("Group23Var7", GroupVariation::Group23Var7)
        .value("Group23Var8", GroupVariation::Group23Var8)
        .value("Group30Var0", GroupVariation::Group30Var0)
        .value("Group30Var1", GroupVariation::Group30Var1)
        .value("Group30Var2", GroupVariation::Group30Var2)
        .value("Group30Var3", GroupVariation::Group30Var3)
        .value("Group30Var4", GroupVariation::Group30Var4)
        .value("Group30Var5", GroupVariation::Group30Var5)
        .value("Group30Var6", GroupVariation::Group30Var6)
        .value("Group31Var0", GroupVariation::Group31Var0)
        .value("Group31Var1", GroupVariation::Group31Var1)
        .value("Group31Var2", GroupVariation::Group31Var2)
        .value("Group31Var3", GroupVariation::Group31Var3)
        .value("Group31Var4", GroupVariation::Group31Var4)
        .value("Group31Var5", GroupVariation::Group31Var5)
        .value("Group31Var6", GroupVariation::Group31Var6)
        .value("Group31Var7", GroupVariation::Group31Var7)
        .value("Group31Var8", GroupVariation::Group31Var8)
        .value("Group32Var0", GroupVariation::Group32Var0)
        .value("Group32Var1", GroupVariation::Group32Var1)
        .value("Group32Var2", GroupVariation::Group32Var2)
        .value("Group32Var3", GroupVariation::Group32Var3)
        .value("Group32Var4", GroupVariation::Group32Var4)
        .value("Group32Var5", GroupVariation::Group32Var5)
        .value("Group32Var6", GroupVariation::Group32Var6)
        .value("Group32Var7", GroupVariation::Group32Var7)
        .value("Group32Var8", GroupVariation::Group32Var8)
        .value("Group33Var0", GroupVariation::Group33Var0)
        .value("Group33Var1", GroupVariation::Group33Var1)
        .value("Group33Var2", GroupVariation::Group33Var2)
        .value("Group33Var3", GroupVariation::Group33Var3)
        .value("Group33Var4", GroupVariation::Group33Var4)
        .value("Group33Var5", GroupVariation::Group33Var5)
        .value("Group33Var6", GroupVariation::Group33Var6)
        .value("Group33Var7", GroupVariation::Group33Var7)
        .value("Group33Var8", GroupVariation::Group33Var8)
        .value("Group34Var0", GroupVariation::Group34Var0)
        .value("Group34Var1", GroupVariation::Group34Var1)
        .value("Group34Var2", GroupVariation::Group34Var2)
        .value("Group34Var3", GroupVariation::Group34Var3)
        .value("Group40Var0", GroupVariation::Group40Var0)
        .value("Group40Var1", GroupVariation::Group40Var1)
        .value("Group40Var2", GroupVariation::Group40Var2)
        .value("Group40Var3", GroupVariation::Group40Var3)
        .value("Group40Var4", GroupVariation::Group40Var4)
        .value("Group41Var0", GroupVariation::Group41Var0)
        .value("Group41Var1", GroupVariation::Group41Var1)
        .value("Group41Var2", GroupVariation::Group41Var2)
        .value("Group41Var3", GroupVariation::Group41Var3)
        .value("Group41Var4", GroupVariation::Group41Var4)
        .value("Group42Var0", GroupVariation::Group42Var0)
        .value("Group42Var1", GroupVariation::Group42Var1)
        .value("Group42Var2", GroupVariation::Group42Var2)
        .value("Group42Var3", GroupVariation::Group42Var3)
        .value("Group42Var4", GroupVariation::Group42Var4)
        .value("Group42Var5", GroupVariation::Group42Var5)
        .value("Group42Var6", GroupVariation::Group42Var6)
        .value("Group42Var7", GroupVariation::Group42Var7)
        .value("Group42Var8", GroupVariation::Group42Var8)
        .value("Group43Var0", GroupVariation::Group43Var0)
        .value("Group43Var1", GroupVariation::Group43Var1)
        .value("Group43Var2", GroupVariation::Group43Var2)
        .value("Group43Var3", GroupVariation::Group43Var3)
        .value("Group43Var4", GroupVariation::Group43Var4)
        .value("Group43Var5", GroupVariation::Group43Var5)
        .value("Group43Var6", GroupVariation::Group43Var6)
        .value("Group43Var7", GroupVariation::Group43Var7)
        .value("Group43Var8", GroupVariation::Group43Var8)
        .value("Group50Var0", GroupVariation::Group50Var0)
        .value("Group50Var1", GroupVariation::Group50Var1)
        .value("Group50Var2", GroupVariation::Group50Var2)
        .value("Group50Var3", GroupVariation::Group50Var3)
        .value("Group50Var4", GroupVariation::Group50Var4)
        .value("Group51Var1", GroupVariation::Group51Var1)
        .value("Group51Var2", GroupVariation::Group51Var2)
        .value("Group52Var1", GroupVariation::Group52Var1)
        .value("Group52Var2", GroupVariation::Group52Var2)
        .value("Group60Var1", GroupVariation::Group60Var1)
        .value("Group60Var2", GroupVariation::Group60Var2)
        .value("Group60Var3", GroupVariation::Group60Var3)
        .value("Group60Var4", GroupVariation::Group60Var4)
        .value("Group70Var1", GroupVariation::Group70Var1)
        .value("Group70Var2", GroupVariation::Group70Var2)
        .value("Group70Var3", GroupVariation::Group70Var3)
        .value("Group70Var4", GroupVariation::Group70Var4)
        .value("Group70Var5", GroupVariation::Group70Var5)
        .value("Group70Var6", GroupVariation::Group70Var6)
        .value("Group70Var7", GroupVariation::Group70Var7)
        .value("Group70Var8", GroupVariation::Group70Var8)
        .value("Group80Var1", GroupVariation::Group80Var1)
        .value("Group85Var1", GroupVariation::Group85Var1)
        .value("Group86Var0", GroupVariation::Group86Var0)
        .value("Group86Var1", GroupVariation::Group86Var1)
        .value("Group86Var2", GroupVariation::Group86Var2)
        .value("Group86Var3", GroupVariation::Group86Var3)
        .value("Group87Var1", GroupVariation::Group87Var1)
        .value("Group88Var1", GroupVariation::Group88Var1)
        .value("Group102Var0", GroupVariation::Group102Var0)
        .value("Group102Var1", GroupVariation::Group102Var1)
        .value("Group110Var0", GroupVariation::Group110Var0)
        .value("Group111Var0", GroupVariation::Group111Var0)
        .value("Group112Var0", GroupVariation::Group112Var0)
        .value("Group113Var0", GroupVariation::Group113Var0)
        .value("Group120Var1", GroupVariation::Group120Var1)
        .value("Group120Var2", GroupVariation::Group120Var2)
        .value("Group120Var3", GroupVariation::Group120Var3)
        .value("Group120Var4", GroupVariation::Group120Var4)
        .value("Group120Var5", GroupVariation::Group120Var5)
        .value("Group120Var6", GroupVariation::Group120Var6)
        .value("Group120Var7", GroupVariation::Group120Var7)
        .value("Group120Var8", GroupVariation::Group120Var8)
        .value("Group120Var9", GroupVariation::Group120Var9)
        .value("Group120Var10", GroupVariation::Group120Var10)
        .value("Group120Var11", GroupVariation::Group120Var11)
        .value("Group120Var12", GroupVariation::Group120Var12)
        .value("Group120Var13", GroupVariation::Group120Var13)
        .value("Group120Var14", GroupVariation::Group120Var14)
        .value("Group120Var15", GroupVariation::Group120Var15)
        .value("Group121Var0", GroupVariation::Group121Var0)
        .value("Group121Var1", GroupVariation::Group121Var1)
        .value("Group122Var0", GroupVariation::Group122Var0)
        .value("Group122Var1", GroupVariation::Group122Var1)
        .value("Group122Var2", GroupVariation::Group122Var2)
        .value("UNKNOWN", GroupVariation::UNKNOWN);

    // QualifierCode
    py::enum_<QualifierCode>(m, "QualifierCode", "Object header range/prefix qualifier codes")
        .value("UINT8_START_STOP", QualifierCode::UINT8_START_STOP)
        .value("UINT16_START_STOP", QualifierCode::UINT16_START_STOP)
        .value("UINT32_START_STOP", QualifierCode::UINT32_START_STOP)
        .value("UINT8_ADDR", QualifierCode::UINT8_ADDR)
        .value("UINT16_ADDR", QualifierCode::UINT16_ADDR)
        .value("UINT32_ADDR", QualifierCode::UINT32_ADDR)
        .value("ALL_OBJECTS", QualifierCode::ALL_OBJECTS)
        .value("UINT8_CNT", QualifierCode::UINT8_CNT)
        .value("UINT16_CNT", QualifierCode::UINT16_CNT)
        .value("UINT32_CNT", QualifierCode::UINT32_CNT)
        .value("UINT8_CNT_UINT8_INDEX", QualifierCode::UINT8_CNT_UINT8_INDEX)
        .value("UINT16_CNT_UINT16_INDEX", QualifierCode::UINT16_CNT_UINT16_INDEX)
        .value("UINT32_CNT_UINT8_INDEX", QualifierCode::UINT32_CNT_UINT8_INDEX)
        .value("UINT8_CNT_UINT16_FREE_FORMAT", QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT)
        .value("UNDEFINED", QualifierCode::UNDEFINED);

    // FileMode
    py::enum_<FileMode>(m, "FileMode", "DNP3 file transfer mode")
        .value("READ", FileMode::READ)
        .value("WRITE", FileMode::WRITE)
        .value("APPEND", FileMode::APPEND);

    // FileStatus
    py::enum_<FileStatus>(m, "FileStatus", "DNP3 file status codes")
        .value("SUCCESS", FileStatus::SUCCESS)
        .value("PERMISSION_DENIED", FileStatus::PERMISSION_DENIED)
        .value("INVALID_MODE", FileStatus::INVALID_MODE)
        .value("FILE_NOT_FOUND", FileStatus::FILE_NOT_FOUND)
        .value("FILE_LOCKED", FileStatus::FILE_LOCKED)
        .value("NOT_OPENED", FileStatus::NOT_OPENED)
        .value("CLOSE_ABORT", FileStatus::CLOSE_ABORT)
        .value("NOT_EXIST", FileStatus::NOT_EXIST)
        .value("HANDLE_EXPIRED", FileStatus::HANDLE_EXPIRED)
        .value("BUFFER_OVERFLOW", FileStatus::BUFFER_OVERFLOW)
        .value("FATAL", FileStatus::FATAL)
        .value("BLOCK_SEQ", FileStatus::BLOCK_SEQ);

    // FileType
    py::enum_<FileType>(m, "FileType", "DNP3 file type field")
        .value("DIRECTORY", FileType::DIRECTORY)
        .value("SIMPLE_FILE", FileType::SIMPLE_FILE);
}
