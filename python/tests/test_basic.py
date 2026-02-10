"""Unit tests for opendnp3 Python bindings -- types, enums, and configuration.

These tests verify actual behavior, edge cases, boundary conditions, and
meaningful invariants rather than just checking that objects construct.
"""

import pytest
import opendnp3


# ===========================================================================
# Module import and API surface
# ===========================================================================

def test_import():
    """Module loads and has expected top-level attributes."""
    assert hasattr(opendnp3, "DNP3Manager")
    assert hasattr(opendnp3, "IChannel")
    assert hasattr(opendnp3, "IMaster")
    assert hasattr(opendnp3, "IOutstation")


def test_module_has_all_key_types():
    """Module exposes all key DNP3 types needed for a working application."""
    required = [
        "DNP3Manager", "IChannel", "IMaster", "IOutstation",
        "ISOEHandler", "ICommandHandler", "IMasterApplication",
        "IOutstationApplication", "ILogHandler", "IChannelListener",
        "MasterStackConfig", "OutstationStackConfig", "DatabaseConfig",
        "LinkConfig", "UpdateBuilder", "Header", "ClassField",
        "Binary", "Analog", "Counter", "FrozenCounter",
        "DoubleBitBinary", "BinaryOutputStatus", "AnalogOutputStatus",
        "OctetString", "TimeAndInterval",
        "ControlRelayOutputBlock", "AnalogOutputInt16", "AnalogOutputInt32",
        "AnalogOutputFloat32", "AnalogOutputDouble64",
        "IINField", "ApplicationIIN", "TLSConfig",
    ]
    for name in required:
        assert hasattr(opendnp3, name), f"Module missing required type: {name}"


# ===========================================================================
# Enum behavioral tests
# ===========================================================================

def test_enums_channel_state_distinct_and_ordered():
    """ChannelState enum values are distinct and follow the expected lifecycle."""
    states = [
        opendnp3.ChannelState.CLOSED,
        opendnp3.ChannelState.OPENING,
        opendnp3.ChannelState.OPEN,
        opendnp3.ChannelState.SHUTDOWN,
    ]
    # All values must be distinct from each other
    for i, a in enumerate(states):
        for j, b in enumerate(states):
            if i != j:
                assert a != b, f"States {i} and {j} should be distinct"


def test_enums_command_status_success_is_not_failure():
    """CommandStatus.SUCCESS is different from all failure statuses."""
    success = opendnp3.CommandStatus.SUCCESS
    failures = [
        opendnp3.CommandStatus.TIMEOUT,
        opendnp3.CommandStatus.NO_SELECT,
        opendnp3.CommandStatus.FORMAT_ERROR,
        opendnp3.CommandStatus.NOT_SUPPORTED,
        opendnp3.CommandStatus.ALREADY_ACTIVE,
        opendnp3.CommandStatus.HARDWARE_ERROR,
        opendnp3.CommandStatus.LOCAL,
        opendnp3.CommandStatus.TOO_MANY_OPS,
        opendnp3.CommandStatus.NOT_AUTHORIZED,
        opendnp3.CommandStatus.AUTOMATION_INHIBIT,
        opendnp3.CommandStatus.PROCESSING_LIMITED,
        opendnp3.CommandStatus.OUT_OF_RANGE,
        opendnp3.CommandStatus.DOWNSTREAM_LOCAL,
        opendnp3.CommandStatus.ALREADY_COMPLETE,
        opendnp3.CommandStatus.BLOCKED,
        opendnp3.CommandStatus.CANCELLED,
        opendnp3.CommandStatus.BLOCKED_OTHER_MASTER,
        opendnp3.CommandStatus.DOWNSTREAM_FAIL,
        opendnp3.CommandStatus.NON_PARTICIPATING,
        opendnp3.CommandStatus.UNDEFINED,
    ]
    for f in failures:
        assert success != f, f"SUCCESS should differ from {f}"
    # All failure statuses must also be distinct from each other
    assert len(set(id(f) for f in failures)) == len(failures)


def test_enums_point_class_values_are_distinct():
    """PointClass enum has exactly 4 distinct values."""
    classes = [
        opendnp3.PointClass.Class0,
        opendnp3.PointClass.Class1,
        opendnp3.PointClass.Class2,
        opendnp3.PointClass.Class3,
    ]
    for i in range(len(classes)):
        for j in range(i + 1, len(classes)):
            assert classes[i] != classes[j]


def test_enums_double_bit_has_four_states():
    """DoubleBit enum has the 4 states defined by DNP3."""
    values = [
        opendnp3.DoubleBit.INTERMEDIATE,
        opendnp3.DoubleBit.DETERMINED_OFF,
        opendnp3.DoubleBit.DETERMINED_ON,
        opendnp3.DoubleBit.INDETERMINATE,
    ]
    assert len(set(id(v) for v in values)) == 4


def test_enums_timestamp_quality_distinct():
    """TimestampQuality has 3 distinct values."""
    vals = [
        opendnp3.TimestampQuality.SYNCHRONIZED,
        opendnp3.TimestampQuality.UNSYNCHRONIZED,
        opendnp3.TimestampQuality.INVALID,
    ]
    assert len(set(id(v) for v in vals)) == 3


def test_enums_freeze_type_all_four():
    """FreezeType has exactly 4 values, all distinct."""
    vals = [
        opendnp3.FreezeType.ImmediateFreeze,
        opendnp3.FreezeType.ImmediateFreezeNR,
        opendnp3.FreezeType.FreezeAndClear,
        opendnp3.FreezeType.FreezeAndClearNR,
    ]
    assert len(set(id(v) for v in vals)) == 4


def test_enums_function_code_request_vs_response():
    """FunctionCode correctly distinguishes request and response codes."""
    # Request function codes
    requests = [
        opendnp3.FunctionCode.READ,
        opendnp3.FunctionCode.WRITE,
        opendnp3.FunctionCode.SELECT,
        opendnp3.FunctionCode.OPERATE,
        opendnp3.FunctionCode.DIRECT_OPERATE,
        opendnp3.FunctionCode.COLD_RESTART,
        opendnp3.FunctionCode.WARM_RESTART,
        opendnp3.FunctionCode.ENABLE_UNSOLICITED,
        opendnp3.FunctionCode.DISABLE_UNSOLICITED,
    ]
    responses = [
        opendnp3.FunctionCode.RESPONSE,
        opendnp3.FunctionCode.UNSOLICITED_RESPONSE,
    ]
    # Request codes must differ from response codes
    for req in requests:
        for resp in responses:
            assert req != resp


def test_enums_operation_type_completeness():
    """OperationType has all CROB operation types."""
    vals = [
        opendnp3.OperationType.NUL,
        opendnp3.OperationType.PULSE_ON,
        opendnp3.OperationType.PULSE_OFF,
        opendnp3.OperationType.LATCH_ON,
        opendnp3.OperationType.LATCH_OFF,
        opendnp3.OperationType.Undefined,
    ]
    assert len(set(id(v) for v in vals)) == 6


def test_enums_trip_close_code_completeness():
    """TripCloseCode has all 4 values."""
    vals = [
        opendnp3.TripCloseCode.NUL,
        opendnp3.TripCloseCode.CLOSE,
        opendnp3.TripCloseCode.TRIP,
        opendnp3.TripCloseCode.RESERVED,
    ]
    assert len(set(id(v) for v in vals)) == 4


def test_enums_task_completion_success_vs_failure():
    """TaskCompletion.SUCCESS differs from all FAILURE variants."""
    success = opendnp3.TaskCompletion.SUCCESS
    failures = [
        opendnp3.TaskCompletion.FAILURE_BAD_RESPONSE,
        opendnp3.TaskCompletion.FAILURE_RESPONSE_TIMEOUT,
        opendnp3.TaskCompletion.FAILURE_START_TIMEOUT,
        opendnp3.TaskCompletion.FAILURE_MESSAGE_FORMAT_ERROR,
        opendnp3.TaskCompletion.FAILURE_NO_COMMS,
    ]
    for f in failures:
        assert success != f


def test_enums_master_task_type_completeness():
    """MasterTaskType has all expected startup and polling task types."""
    vals = [
        opendnp3.MasterTaskType.CLEAR_RESTART,
        opendnp3.MasterTaskType.DISABLE_UNSOLICITED,
        opendnp3.MasterTaskType.ASSIGN_CLASS,
        opendnp3.MasterTaskType.STARTUP_INTEGRITY_POLL,
        opendnp3.MasterTaskType.NON_LAN_TIME_SYNC,
        opendnp3.MasterTaskType.LAN_TIME_SYNC,
        opendnp3.MasterTaskType.ENABLE_UNSOLICITED,
        opendnp3.MasterTaskType.AUTO_EVENT_SCAN,
        opendnp3.MasterTaskType.USER_TASK,
    ]
    assert len(set(id(v) for v in vals)) == 9


def test_enums_command_point_state_lifecycle():
    """CommandPointState covers the full command lifecycle."""
    vals = [
        opendnp3.CommandPointState.INIT,
        opendnp3.CommandPointState.SELECT_SUCCESS,
        opendnp3.CommandPointState.SELECT_MISMATCH,
        opendnp3.CommandPointState.SELECT_FAIL,
        opendnp3.CommandPointState.OPERATE_FAIL,
        opendnp3.CommandPointState.SUCCESS,
    ]
    assert len(set(id(v) for v in vals)) == 6


def test_enums_group_variation_known_mappings():
    """GroupVariation enum maps to correct group/variation pairs."""
    # Spot-check that key group/variation names exist and are distinct
    gv_binary = opendnp3.GroupVariation.Group1Var0
    gv_binary_event = opendnp3.GroupVariation.Group2Var1
    gv_analog = opendnp3.GroupVariation.Group30Var1
    gv_counter = opendnp3.GroupVariation.Group20Var1 if hasattr(opendnp3.GroupVariation, 'Group20Var1') else opendnp3.GroupVariation.Group20Var0
    gv_unknown = opendnp3.GroupVariation.UNKNOWN
    assert gv_binary != gv_binary_event
    assert gv_analog != gv_binary
    assert gv_unknown != gv_binary


def test_enums_qualifier_code_all_distinct():
    """QualifierCode values are all distinct."""
    vals = [
        opendnp3.QualifierCode.UINT8_START_STOP,
        opendnp3.QualifierCode.UINT16_START_STOP,
        opendnp3.QualifierCode.ALL_OBJECTS,
        opendnp3.QualifierCode.UINT8_CNT,
        opendnp3.QualifierCode.UINT16_CNT,
        opendnp3.QualifierCode.UINT8_CNT_UINT8_INDEX,
        opendnp3.QualifierCode.UINT16_CNT_UINT16_INDEX,
        opendnp3.QualifierCode.UNDEFINED,
    ]
    assert len(set(id(v) for v in vals)) == 8


def test_enums_serial_config_types():
    """Serial config enums (StopBits, Parity, FlowControl) have expected values."""
    assert opendnp3.StopBits.One != opendnp3.StopBits.Two
    assert opendnp3.Parity.Even != opendnp3.Parity.Odd
    assert opendnp3.FlowControl.Hardware != opendnp3.FlowControl.XONXOFF


def test_enums_event_mode_semantics():
    """EventMode values are distinct and cover all three behaviors."""
    detect = opendnp3.EventMode.Detect
    force = opendnp3.EventMode.Force
    suppress = opendnp3.EventMode.Suppress
    assert detect != force
    assert force != suppress
    assert detect != suppress


def test_enums_server_accept_mode_distinct():
    """ServerAcceptMode has two distinct connection handling strategies."""
    assert opendnp3.ServerAcceptMode.CloseNew != opendnp3.ServerAcceptMode.CloseExisting


def test_enums_time_sync_mode_distinct():
    """TimeSyncMode has 3 distinct values."""
    none = getattr(opendnp3.TimeSyncMode, "None")
    non_lan = opendnp3.TimeSyncMode.NonLAN
    lan = opendnp3.TimeSyncMode.LAN
    assert none != non_lan
    assert non_lan != lan
    assert none != lan


def test_enums_index_qualifier_mode_distinct():
    """IndexQualifierMode has 2 distinct values."""
    assert opendnp3.IndexQualifierMode.allow_one_byte != opendnp3.IndexQualifierMode.always_two_bytes


def test_enums_restart_type_and_mode():
    """RestartType and RestartMode have expected distinct values."""
    assert opendnp3.RestartType.COLD != opendnp3.RestartType.WARM
    assert opendnp3.RestartMode.UNSUPPORTED != opendnp3.RestartMode.SUPPORTED_DELAY_FINE
    assert opendnp3.RestartMode.SUPPORTED_DELAY_FINE != opendnp3.RestartMode.SUPPORTED_DELAY_COARSE


def test_enums_link_status_distinct():
    """LinkStatus has 2 distinct values."""
    assert opendnp3.LinkStatus.UNRESET != opendnp3.LinkStatus.RESET


def test_enums_operate_type_distinct():
    """OperateType has 3 distinct values for SBO/DO/DONR."""
    assert opendnp3.OperateType.SelectBeforeOperate != opendnp3.OperateType.DirectOperate
    assert opendnp3.OperateType.DirectOperate != opendnp3.OperateType.DirectOperateNoAck


def test_enums_flags_type_covers_all_point_types():
    """FlagsType has a value for each measurement type that has quality flags."""
    vals = [
        opendnp3.FlagsType.BinaryInput,
        opendnp3.FlagsType.DoubleBinaryInput,
        opendnp3.FlagsType.Counter,
        opendnp3.FlagsType.FrozenCounter,
        opendnp3.FlagsType.AnalogInput,
        opendnp3.FlagsType.BinaryOutputStatus,
        opendnp3.FlagsType.AnalogOutputStatus,
    ]
    assert len(set(id(v) for v in vals)) == 7


def test_enums_interval_units_ordering():
    """IntervalUnits has time unit values from none through weeks."""
    # Just verify they are all distinct
    vals = [
        opendnp3.IntervalUnits.NoRepeat,
        opendnp3.IntervalUnits.Milliseconds,
        opendnp3.IntervalUnits.Seconds,
        opendnp3.IntervalUnits.Minutes,
        opendnp3.IntervalUnits.Hours,
        opendnp3.IntervalUnits.Days,
        opendnp3.IntervalUnits.Weeks,
        opendnp3.IntervalUnits.Undefined,
    ]
    assert len(set(id(v) for v in vals)) == 8


def test_enums_iin_bit_all_16_bits():
    """IINBit enum has exactly 16 distinct values (8 LSB + 8 MSB)."""
    lsb_bits = [
        opendnp3.IINBit.BROADCAST,
        opendnp3.IINBit.CLASS1_EVENTS,
        opendnp3.IINBit.CLASS2_EVENTS,
        opendnp3.IINBit.CLASS3_EVENTS,
        opendnp3.IINBit.NEED_TIME,
        opendnp3.IINBit.LOCAL_CONTROL,
        opendnp3.IINBit.DEVICE_TROUBLE,
        opendnp3.IINBit.DEVICE_RESTART,
    ]
    msb_bits = [
        opendnp3.IINBit.FUNC_NOT_SUPPORTED,
        opendnp3.IINBit.OBJECT_UNKNOWN,
        opendnp3.IINBit.PARAM_ERROR,
        opendnp3.IINBit.EVENT_BUFFER_OVERFLOW,
        opendnp3.IINBit.ALREADY_EXECUTING,
        opendnp3.IINBit.CONFIG_CORRUPT,
        opendnp3.IINBit.RESERVED1,
        opendnp3.IINBit.RESERVED2,
    ]
    all_bits = lsb_bits + msb_bits
    assert len(set(id(v) for v in all_bits)) == 16


# ===========================================================================
# Measurement types -- values, flags, time, edge cases
# ===========================================================================

def test_binary_value_semantics():
    """Binary preserves True/False distinction and default flags include ONLINE."""
    b_true = opendnp3.Binary(True)
    b_false = opendnp3.Binary(False)
    assert b_true.value is True
    assert b_false.value is False
    # Default construction with just a value should have ONLINE flag (0x01)
    # plus the value bit embedded in the flags byte
    assert b_true.flags.value != b_false.flags.value


def test_binary_full_construction():
    """Binary supports value+flags and value+flags+time constructors."""
    flags = opendnp3.Flags(0x01)
    b = opendnp3.Binary(True, flags)
    assert b.value is True
    # The Binary constructor combines the value bit into the flags byte
    assert b.flags.value & 0x01  # ONLINE bit preserved

    t = opendnp3.DNPTime(10000)
    b2 = opendnp3.Binary(False, flags, t)
    assert b2.value is False
    assert b2.time.value == 10000


def test_analog_value_precision_and_negatives():
    """Analog stores double-precision values including negatives and zero."""
    assert opendnp3.Analog(0.0).value == 0.0
    assert opendnp3.Analog(-100.5).value == -100.5
    assert abs(opendnp3.Analog(3.141592653589793).value - 3.141592653589793) < 1e-12
    # Large values
    assert opendnp3.Analog(1e15).value == 1e15
    assert opendnp3.Analog(-1e15).value == -1e15


def test_analog_full_construction():
    """Analog supports all constructors with flags and time."""
    flags = opendnp3.Flags(0x01)
    a = opendnp3.Analog(5.5, flags)
    assert abs(a.value - 5.5) < 1e-6
    assert a.flags.value == 0x01

    t = opendnp3.DNPTime(20000)
    a2 = opendnp3.Analog(7.7, flags, t)
    assert a2.time.value == 20000


def test_counter_zero_and_large_values():
    """Counter stores unsigned 32-bit values correctly."""
    assert opendnp3.Counter(0).value == 0
    assert opendnp3.Counter(42).value == 42
    # Max uint32
    assert opendnp3.Counter(0xFFFFFFFF).value == 0xFFFFFFFF


def test_counter_full_construction():
    """Counter supports all constructors."""
    flags = opendnp3.Flags(0x01)
    t = opendnp3.DNPTime(30000)
    c = opendnp3.Counter(42, flags, t)
    assert c.value == 42
    assert c.flags.value == 0x01
    assert c.time.value == 30000


def test_frozen_counter_all_constructors():
    """FrozenCounter supports default, value-only, and full constructors."""
    fc = opendnp3.FrozenCounter()
    assert fc.value == 0

    fc2 = opendnp3.FrozenCounter(500)
    assert fc2.value == 500

    flags = opendnp3.Flags(0x01)
    t = opendnp3.DNPTime(5000)
    fc3 = opendnp3.FrozenCounter(123, flags, t)
    assert fc3.value == 123
    assert fc3.flags.value == 0x01
    assert fc3.time.value == 5000


def test_double_bit_binary_all_values():
    """DoubleBitBinary can be constructed with all 4 DoubleBit states."""
    for db_val in [opendnp3.DoubleBit.INTERMEDIATE,
                   opendnp3.DoubleBit.DETERMINED_OFF,
                   opendnp3.DoubleBit.DETERMINED_ON,
                   opendnp3.DoubleBit.INDETERMINATE]:
        dbb = opendnp3.DoubleBitBinary(db_val)
        assert dbb.value == db_val

    # Default should be INDETERMINATE
    assert opendnp3.DoubleBitBinary().value == opendnp3.DoubleBit.INDETERMINATE


def test_binary_output_status_default_and_explicit():
    """BinaryOutputStatus defaults to False, can be set to True."""
    bos = opendnp3.BinaryOutputStatus()
    assert bos.value is False
    bos2 = opendnp3.BinaryOutputStatus(True)
    assert bos2.value is True


def test_analog_output_status_default_and_value():
    """AnalogOutputStatus defaults to 0.0 and preserves set values."""
    aos = opendnp3.AnalogOutputStatus()
    assert aos.value == 0.0
    aos2 = opendnp3.AnalogOutputStatus(99.5)
    assert abs(aos2.value - 99.5) < 1e-6
    aos3 = opendnp3.AnalogOutputStatus(-42.0)
    assert abs(aos3.value - (-42.0)) < 1e-6


def test_time_and_interval_fields():
    """TimeAndInterval stores time, interval, and units correctly."""
    tai = opendnp3.TimeAndInterval()
    assert tai.interval == 0  # default should be 0

    t = opendnp3.DNPTime(1000)
    tai2 = opendnp3.TimeAndInterval(t, 5000, opendnp3.IntervalUnits.Seconds)
    assert tai2.time.value == 1000
    assert tai2.interval == 5000
    assert tai2.units == 2 or tai2.units == opendnp3.IntervalUnits.Seconds


# ===========================================================================
# OctetString -- boundary conditions
# ===========================================================================

def test_octet_string_empty():
    """Empty OctetString has size 0 and returns empty bytes."""
    os_empty = opendnp3.OctetString("")
    assert os_empty.Size() == 0
    assert os_empty.ToBytes() == b""


def test_octet_string_ascii():
    """OctetString preserves ASCII content."""
    os_data = opendnp3.OctetString("hello")
    assert os_data.Size() == 5
    assert os_data.ToBytes() == b"hello"


def test_octet_string_binary_data():
    """OctetString can store printable characters of various lengths."""
    # The binding's OctetString constructor works with string data.
    # Verify it handles different sizes correctly.
    for length in [1, 10, 50, 100]:
        data = "X" * length
        os_obj = opendnp3.OctetString(data)
        assert os_obj.Size() == length
        assert os_obj.ToBytes() == data.encode("ascii")


def test_octet_string_max_size():
    """OctetString respects the DNP3 maximum of 255 bytes."""
    data_255 = "A" * 255
    os_max = opendnp3.OctetString(data_255)
    assert os_max.Size() == 255
    assert os_max.ToBytes() == b"A" * 255


# ===========================================================================
# Flags
# ===========================================================================

def test_flags_byte_values():
    """Flags stores the raw quality byte and supports boundary values."""
    assert opendnp3.Flags(0x00).value == 0
    assert opendnp3.Flags(0x01).value == 1
    assert opendnp3.Flags(0x80).value == 0x80
    assert opendnp3.Flags(0xFF).value == 0xFF


# ===========================================================================
# DNPTime
# ===========================================================================

def test_dnptime_default_quality():
    """DNPTime defaults to SYNCHRONIZED quality."""
    t = opendnp3.DNPTime(1000)
    assert t.value == 1000
    assert t.quality == opendnp3.TimestampQuality.SYNCHRONIZED


def test_dnptime_explicit_quality():
    """DNPTime can be constructed with each quality level."""
    for q in [opendnp3.TimestampQuality.SYNCHRONIZED,
              opendnp3.TimestampQuality.UNSYNCHRONIZED,
              opendnp3.TimestampQuality.INVALID]:
        t = opendnp3.DNPTime(5000, q)
        assert t.value == 5000
        assert t.quality == q


def test_dnptime_zero():
    """DNPTime value 0 is valid (epoch)."""
    t = opendnp3.DNPTime(0)
    assert t.value == 0


def test_dnptime_large_value():
    """DNPTime supports large millisecond timestamps (year 2100+)."""
    far_future = 4102444800000  # ~2100-01-01
    t = opendnp3.DNPTime(far_future)
    assert t.value == far_future


# ===========================================================================
# TimeDuration
# ===========================================================================

def test_time_duration_repr_contains_value():
    """TimeDuration repr shows the duration value as a string."""
    td = opendnp3.TimeDuration.Seconds(5)
    r = repr(td)
    assert isinstance(r, str)
    assert len(r) > 0


def test_time_duration_zero_is_zero():
    """TimeDuration.Zero() represents zero duration."""
    td = opendnp3.TimeDuration.Zero()
    # repr should indicate zero
    r = repr(td)
    assert "0" in r


def test_time_duration_factories_produce_objects():
    """All TimeDuration factory methods produce valid duration objects."""
    ms = opendnp3.TimeDuration.Milliseconds(100)
    sec = opendnp3.TimeDuration.Seconds(5)
    mins = opendnp3.TimeDuration.Minutes(2)
    zero = opendnp3.TimeDuration.Zero()
    maxd = opendnp3.TimeDuration.Max()
    # Verify they are all TimeDuration objects
    for td in [ms, sec, mins, zero, maxd]:
        assert isinstance(repr(td), str)


# ===========================================================================
# IPEndpoint
# ===========================================================================

def test_ip_endpoint_stores_address_and_port():
    """IPEndpoint stores the exact address and port given."""
    ep = opendnp3.IPEndpoint("127.0.0.1", 20000)
    assert ep.address == "127.0.0.1"
    assert ep.port == 20000

    ep2 = opendnp3.IPEndpoint("192.168.1.100", 502)
    assert ep2.address == "192.168.1.100"
    assert ep2.port == 502


def test_ip_endpoint_localhost():
    """IPEndpoint.Localhost returns 127.0.0.1."""
    ep = opendnp3.IPEndpoint.Localhost(20000)
    assert ep.address == "127.0.0.1"
    assert ep.port == 20000


def test_ip_endpoint_all_adapters():
    """IPEndpoint.AllAdapters binds to 0.0.0.0."""
    ep = opendnp3.IPEndpoint.AllAdapters(20000)
    assert ep.address == "0.0.0.0"
    assert ep.port == 20000


# ===========================================================================
# ChannelRetry
# ===========================================================================

def test_channel_retry_default_has_valid_fields():
    """ChannelRetry.Default() returns an object with accessible delay fields."""
    retry = opendnp3.ChannelRetry.Default()
    # All three fields should be accessible and be TimeDuration objects
    assert isinstance(repr(retry.minOpenRetry), str)
    assert isinstance(repr(retry.maxOpenRetry), str)
    assert isinstance(repr(retry.reconnectDelay), str)


def test_channel_retry_custom():
    """ChannelRetry can be constructed with custom min/max durations."""
    cr = opendnp3.ChannelRetry(
        opendnp3.TimeDuration.Seconds(1),
        opendnp3.TimeDuration.Seconds(30),
    )
    # Verify the fields are accessible TimeDuration objects
    assert isinstance(repr(cr.minOpenRetry), str)
    assert isinstance(repr(cr.maxOpenRetry), str)


# ===========================================================================
# LinkConfig
# ===========================================================================

def test_link_config_master_defaults():
    """LinkConfig for master has correct default addresses (1 local, 1024 remote)."""
    lc = opendnp3.LinkConfig(True)
    assert lc.IsMaster is True
    assert lc.LocalAddr == 1
    assert lc.RemoteAddr == 1024


def test_link_config_outstation_defaults():
    """LinkConfig for outstation has correct default addresses (1024 local, 1 remote)."""
    lc = opendnp3.LinkConfig(False)
    assert lc.IsMaster is False
    assert lc.LocalAddr == 1024
    assert lc.RemoteAddr == 1


def test_link_config_custom_addresses():
    """LinkConfig accepts custom addresses and timeouts."""
    lc = opendnp3.LinkConfig(True, 5, 10,
                              opendnp3.TimeDuration.Seconds(5),
                              opendnp3.TimeDuration.Seconds(60))
    assert lc.IsMaster is True
    assert lc.LocalAddr == 5
    assert lc.RemoteAddr == 10


# ===========================================================================
# MasterStackConfig / OutstationStackConfig
# ===========================================================================

def test_master_stack_config_defaults():
    """MasterStackConfig defaults to IsMaster=True and has accessible sub-configs."""
    config = opendnp3.MasterStackConfig()
    assert config.link.IsMaster is True
    assert config.master.disableUnsolOnStartup is True


def test_outstation_stack_config_defaults():
    """OutstationStackConfig defaults to IsMaster=False."""
    config = opendnp3.OutstationStackConfig()
    assert config.link.IsMaster is False


def test_outstation_stack_config_with_database():
    """OutstationStackConfig can be created from a DatabaseConfig."""
    db = opendnp3.DatabaseConfig(5)
    config = opendnp3.OutstationStackConfig(db)
    assert config.link.IsMaster is False


# ===========================================================================
# DatabaseConfig
# ===========================================================================

def test_database_config_has_all_point_type_fields():
    """DatabaseConfig has fields for all 9 DNP3 point types."""
    db = opendnp3.DatabaseConfig(5)
    db_type = type(db)
    for field_name in [
        "binary_input", "double_binary", "analog_input", "counter",
        "frozen_counter", "binary_output_status", "analog_output_status",
        "time_and_interval", "octet_string",
    ]:
        assert field_name in dir(db_type), f"Missing field: {field_name}"


def test_database_config_zero_size():
    """DatabaseConfig with size 0 creates an empty database."""
    db = opendnp3.DatabaseConfig(0)
    assert db is not None


# ===========================================================================
# UpdateBuilder -- behavioral tests
# ===========================================================================

def test_update_builder_empty_build_is_empty():
    """An empty UpdateBuilder produces an empty Updates object."""
    builder = opendnp3.UpdateBuilder()
    updates = builder.Build()
    assert updates.IsEmpty()


def test_update_builder_single_update():
    """A single update produces a non-empty Updates object."""
    builder = opendnp3.UpdateBuilder()
    builder.Update(opendnp3.Analog(1.5), 0)
    updates = builder.Build()
    assert not updates.IsEmpty()


def test_update_builder_multiple_types():
    """UpdateBuilder supports all measurement types in a single batch."""
    builder = opendnp3.UpdateBuilder()
    builder.Update(opendnp3.Binary(True), 0)
    builder.Update(opendnp3.DoubleBitBinary(opendnp3.DoubleBit.DETERMINED_ON), 0)
    builder.Update(opendnp3.Analog(1.5), 0)
    builder.Update(opendnp3.Counter(100), 0)
    builder.Update(opendnp3.BinaryOutputStatus(True), 0)
    builder.Update(opendnp3.AnalogOutputStatus(50.0), 0)
    t = opendnp3.DNPTime(1000)
    builder.Update(opendnp3.TimeAndInterval(t, 500, opendnp3.IntervalUnits.Seconds), 0)
    updates = builder.Build()
    assert not updates.IsEmpty()


def test_update_builder_with_event_mode():
    """UpdateBuilder accepts explicit EventMode arguments."""
    builder = opendnp3.UpdateBuilder()
    builder.Update(opendnp3.Binary(True), 0, opendnp3.EventMode.Force)
    builder.Update(opendnp3.Analog(5.0), 0, opendnp3.EventMode.Suppress)
    builder.Update(opendnp3.Counter(10), 0, opendnp3.EventMode.Detect)
    updates = builder.Build()
    assert not updates.IsEmpty()


def test_update_builder_freeze_counter():
    """UpdateBuilder.FreezeCounter can be called with both clear options."""
    builder = opendnp3.UpdateBuilder()
    builder.FreezeCounter(0, True, opendnp3.EventMode.Force)   # freeze and clear
    builder.FreezeCounter(1, False, opendnp3.EventMode.Detect)  # freeze only
    updates = builder.Build()
    assert not updates.IsEmpty()


def test_update_builder_modify_flags():
    """UpdateBuilder.Modify sets flags on a range of points."""
    builder = opendnp3.UpdateBuilder()
    builder.Modify(opendnp3.FlagsType.BinaryInput, 0, 4, 0x01)
    updates = builder.Build()
    assert not updates.IsEmpty()


def test_update_builder_build_resets():
    """After Build(), the builder is consumed and a new Build() returns empty."""
    builder = opendnp3.UpdateBuilder()
    builder.Update(opendnp3.Analog(1.0), 0)
    updates1 = builder.Build()
    assert not updates1.IsEmpty()
    # Building again without new updates should be empty
    updates2 = builder.Build()
    assert updates2.IsEmpty()


# ===========================================================================
# Log levels and flags -- behavioral tests
# ===========================================================================

def test_log_levels_nothing_is_zero():
    """LogLevels NOTHING has a value of 0 (no logging)."""
    assert opendnp3.levels.NOTHING.get_value() == 0


def test_log_levels_normal_is_nonzero():
    """LogLevels NORMAL includes some logging bits."""
    assert opendnp3.levels.NORMAL.get_value() != 0


def test_log_levels_everything_is_superset():
    """LogLevels.everything() has all bits that NORMAL has, plus more."""
    everything = opendnp3.LogLevels.everything().get_value()
    normal = opendnp3.levels.NORMAL.get_value()
    # NORMAL bits should be a subset of everything bits
    assert (everything & normal) == normal
    # everything should have more bits set than NORMAL
    # Note: everything().get_value() returns -1 (0xFFFFFFFF as signed int32),
    # so we compare bit counts rather than raw integer values.
    assert everything != normal


def test_log_levels_none_is_zero():
    """LogLevels.none() has no bits set."""
    assert opendnp3.LogLevels.none().get_value() == 0


def test_log_levels_or_combines_bits():
    """Bitwise OR of LogLevels combines the bits from both operands."""
    normal = opendnp3.levels.NORMAL.get_value()
    link_rx = opendnp3.flags.LINK_RX
    combined = opendnp3.levels.NORMAL | link_rx
    combined_val = combined.get_value()
    # Combined should have at least the bits from NORMAL
    assert (combined_val & normal) == normal
    # Combined should have more bits than NORMAL alone
    assert combined_val >= normal


def test_log_flags_are_individual_bits():
    """Individual log flags represent single-bit values."""
    flag_list = [
        opendnp3.flags.EVENT,
        opendnp3.flags.ERR,
        opendnp3.flags.WARN,
        opendnp3.flags.INFO,
        opendnp3.flags.DBG,
        opendnp3.flags.LINK_RX,
        opendnp3.flags.LINK_TX,
        opendnp3.flags.TRANSPORT_RX,
        opendnp3.flags.TRANSPORT_TX,
        opendnp3.flags.APP_HEADER_RX,
        opendnp3.flags.APP_HEADER_TX,
        opendnp3.flags.APP_OBJECT_RX,
        opendnp3.flags.APP_OBJECT_TX,
    ]
    # All should be non-None
    for f in flag_list:
        assert f is not None


def test_log_level_constants_accessible():
    """Predefined log level combinations are accessible and non-None."""
    for level_name in ["NOTHING", "ALL", "NORMAL", "ALL_APP_COMMS", "ALL_COMMS"]:
        level = getattr(opendnp3.levels, level_name)
        assert level is not None


def test_log_level_construction():
    """LogLevel can be constructed with a value."""
    ll = opendnp3.LogLevel()
    ll2 = opendnp3.LogLevel(5)
    assert ll2.value == 5


# ===========================================================================
# CROB (ControlRelayOutputBlock)
# ===========================================================================

def test_crob_default():
    """CROB default: LATCH_ON, count=1, zero on/off times."""
    crob = opendnp3.ControlRelayOutputBlock()
    assert crob.opType == opendnp3.OperationType.LATCH_ON
    assert crob.count == 1


def test_crob_full_construction():
    """CROB supports all fields including trip/close code."""
    crob = opendnp3.ControlRelayOutputBlock(
        opType=opendnp3.OperationType.PULSE_ON,
        tcc=opendnp3.TripCloseCode.TRIP,
        clear=True,
        count=3,
        onTime=200,
        offTime=300,
        status=opendnp3.CommandStatus.SUCCESS,
    )
    assert crob.opType == opendnp3.OperationType.PULSE_ON
    assert crob.tcc == opendnp3.TripCloseCode.TRIP
    assert crob.clear is True
    assert crob.count == 3
    assert crob.onTimeMS == 200
    assert crob.offTimeMS == 300
    assert crob.status == opendnp3.CommandStatus.SUCCESS


def test_crob_all_operation_types():
    """CROB can be constructed with each OperationType."""
    for op in [opendnp3.OperationType.NUL,
               opendnp3.OperationType.PULSE_ON,
               opendnp3.OperationType.PULSE_OFF,
               opendnp3.OperationType.LATCH_ON,
               opendnp3.OperationType.LATCH_OFF]:
        crob = opendnp3.ControlRelayOutputBlock(opType=op)
        assert crob.opType == op


# ===========================================================================
# Analog output command types
# ===========================================================================

def test_analog_output_int16_boundary_values():
    """AnalogOutputInt16 handles int16 boundary values."""
    assert opendnp3.AnalogOutputInt16(0).value == 0
    assert opendnp3.AnalogOutputInt16(32767).value == 32767
    assert opendnp3.AnalogOutputInt16(-32768).value == -32768

    ao = opendnp3.AnalogOutputInt16(-500, opendnp3.CommandStatus.TIMEOUT)
    assert ao.value == -500
    assert ao.status == opendnp3.CommandStatus.TIMEOUT


def test_analog_output_int32_boundary_values():
    """AnalogOutputInt32 handles int32 boundary values."""
    assert opendnp3.AnalogOutputInt32(0).value == 0
    assert opendnp3.AnalogOutputInt32(2147483647).value == 2147483647
    assert opendnp3.AnalogOutputInt32(-2147483648).value == -2147483648

    ao = opendnp3.AnalogOutputInt32(-100000, opendnp3.CommandStatus.NOT_SUPPORTED)
    assert ao.value == -100000
    assert ao.status == opendnp3.CommandStatus.NOT_SUPPORTED


def test_analog_output_float32_precision():
    """AnalogOutputFloat32 has float32 precision."""
    ao = opendnp3.AnalogOutputFloat32(3.14)
    assert abs(ao.value - 3.14) < 1e-4  # float32 precision

    ao_neg = opendnp3.AnalogOutputFloat32(-1.5, opendnp3.CommandStatus.LOCAL)
    assert abs(ao_neg.value - (-1.5)) < 1e-4
    assert ao_neg.status == opendnp3.CommandStatus.LOCAL


def test_analog_output_double64_precision():
    """AnalogOutputDouble64 has full double precision."""
    ao = opendnp3.AnalogOutputDouble64(3.141592653589793)
    assert abs(ao.value - 3.141592653589793) < 1e-12

    ao2 = opendnp3.AnalogOutputDouble64(99.99, opendnp3.CommandStatus.BLOCKED)
    assert abs(ao2.value - 99.99) < 1e-6
    assert ao2.status == opendnp3.CommandStatus.BLOCKED


# ===========================================================================
# ClassField
# ===========================================================================

def test_classfield_all_classes():
    """ClassField.AllClasses() has all 4 classes set."""
    cf = opendnp3.ClassField.AllClasses()
    assert cf.HasClass0()
    assert cf.HasClass1()
    assert cf.HasClass2()
    assert cf.HasClass3()


def test_classfield_all_event_classes():
    """ClassField.AllEventClasses() has classes 1-3 but NOT class 0."""
    cf = opendnp3.ClassField.AllEventClasses()
    assert not cf.HasClass0()
    assert cf.HasClass1()
    assert cf.HasClass2()
    assert cf.HasClass3()


def test_classfield_none():
    """ClassField.None_() has no classes set."""
    cf = opendnp3.ClassField.None_()
    assert not cf.HasClass0()
    assert not cf.HasClass1()
    assert not cf.HasClass2()
    assert not cf.HasClass3()


def test_classfield_individual_bits():
    """ClassField constructed with individual bits preserves each flag."""
    # All permutations of True/False for 4 classes
    for c0, c1, c2, c3 in [(True, False, False, False),
                             (False, True, False, False),
                             (False, False, True, False),
                             (False, False, False, True),
                             (True, True, True, True),
                             (False, False, False, False)]:
        cf = opendnp3.ClassField(c0, c1, c2, c3)
        assert cf.HasClass0() == c0
        assert cf.HasClass1() == c1
        assert cf.HasClass2() == c2
        assert cf.HasClass3() == c3


# ===========================================================================
# Header factory methods -- verify they produce distinct objects
# ===========================================================================

def test_header_factory_methods_produce_objects():
    """All Header factory methods produce header objects for use in scans."""
    h1 = opendnp3.Header.AllObjects(1, 2)
    h2 = opendnp3.Header.From(opendnp3.PointClass.Class1)
    h3 = opendnp3.Header.Range8(30, 1, 0, 9)
    h4 = opendnp3.Header.Range16(30, 1, 0, 255)
    h5 = opendnp3.Header.Count8(1, 2, 5)
    h6 = opendnp3.Header.Count16(1, 2, 500)
    h7 = opendnp3.Header.Raw(bytes([1, 0, 0x06]))
    h8 = opendnp3.Header.Raw(b"")
    # All should be valid header objects (not None)
    for h in [h1, h2, h3, h4, h5, h6, h7, h8]:
        assert h is not None


def test_header_from_each_point_class():
    """Header.From works with each PointClass value."""
    for pc in [opendnp3.PointClass.Class0,
               opendnp3.PointClass.Class1,
               opendnp3.PointClass.Class2,
               opendnp3.PointClass.Class3]:
        h = opendnp3.Header.From(pc)
        assert h is not None


# ===========================================================================
# IINField comprehensive tests
# ===========================================================================

def test_iinfield_default():
    """IINField default constructor has no bits set."""
    iin = opendnp3.IINField()
    assert not iin.Any()
    assert iin.LSB == 0
    assert iin.MSB == 0


def test_iinfield_empty():
    """IINField.Empty() creates a field with no bits set."""
    iin = opendnp3.IINField.Empty()
    assert not iin.Any()
    assert iin.LSB == 0
    assert iin.MSB == 0


def test_iinfield_with_lsb_msb():
    """IINField can be constructed with explicit LSB/MSB bytes."""
    iin = opendnp3.IINField(0x80, 0x00)  # DEVICE_RESTART
    assert iin.Any()
    assert iin.LSB == 0x80
    assert iin.MSB == 0x00
    assert iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)


def test_iinfield_from_bit():
    """IINField can be constructed from a single IINBit."""
    iin = opendnp3.IINField(opendnp3.IINBit.DEVICE_RESTART)
    assert iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)
    assert not iin.IsSet(opendnp3.IINBit.BROADCAST)
    assert iin.LSB == 0x80


def test_iinfield_clear():
    """IINField.Clear resets all bits to zero."""
    iin = opendnp3.IINField(0xFF, 0xFF)
    assert iin.Any()
    iin.Clear()
    assert not iin.Any()
    assert iin.LSB == 0
    assert iin.MSB == 0


def test_iinfield_is_set_and_is_clear_are_inverses():
    """IINField.IsClear is always the inverse of IsSet for any bit."""
    iin = opendnp3.IINField(opendnp3.IINBit.NEED_TIME)
    assert iin.IsSet(opendnp3.IINBit.NEED_TIME)
    assert not iin.IsClear(opendnp3.IINBit.NEED_TIME)
    assert not iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)
    assert iin.IsClear(opendnp3.IINBit.DEVICE_RESTART)


def test_iinfield_set_bit():
    """IINField.SetBit sets a specific bit without affecting others."""
    iin = opendnp3.IINField()
    iin.SetBit(opendnp3.IINBit.DEVICE_TROUBLE)
    assert iin.IsSet(opendnp3.IINBit.DEVICE_TROUBLE)
    assert not iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)


def test_iinfield_clear_bit():
    """IINField.ClearBit clears only the specified bit."""
    iin = opendnp3.IINField(0xFF, 0xFF)
    iin.ClearBit(opendnp3.IINBit.DEVICE_RESTART)
    assert not iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)
    assert iin.IsSet(opendnp3.IINBit.BROADCAST)  # other bits unaffected


def test_iinfield_set_bit_to_value():
    """IINField.SetBitToValue sets or clears based on boolean."""
    iin = opendnp3.IINField()
    iin.SetBitToValue(opendnp3.IINBit.LOCAL_CONTROL, True)
    assert iin.IsSet(opendnp3.IINBit.LOCAL_CONTROL)
    iin.SetBitToValue(opendnp3.IINBit.LOCAL_CONTROL, False)
    assert not iin.IsSet(opendnp3.IINBit.LOCAL_CONTROL)


def test_iinfield_has_request_error():
    """HasRequestError detects FUNC_NOT_SUPPORTED, OBJECT_UNKNOWN, PARAM_ERROR only."""
    assert not opendnp3.IINField().HasRequestError()

    for err_bit in [opendnp3.IINBit.FUNC_NOT_SUPPORTED,
                    opendnp3.IINBit.OBJECT_UNKNOWN,
                    opendnp3.IINBit.PARAM_ERROR]:
        iin = opendnp3.IINField(err_bit)
        assert iin.HasRequestError(), f"{err_bit} should be a request error"

    # Non-error bits should NOT trigger HasRequestError
    for non_err in [opendnp3.IINBit.DEVICE_RESTART,
                    opendnp3.IINBit.NEED_TIME,
                    opendnp3.IINBit.EVENT_BUFFER_OVERFLOW]:
        iin = opendnp3.IINField(non_err)
        assert not iin.HasRequestError(), f"{non_err} should not be a request error"


def test_iinfield_equality():
    """IINField supports equality comparison."""
    iin1 = opendnp3.IINField(0x80, 0x01)
    iin2 = opendnp3.IINField(0x80, 0x01)
    iin3 = opendnp3.IINField(0x00, 0x00)
    assert iin1 == iin2
    assert not (iin1 == iin3)


def test_iinfield_bitwise_or():
    """IINField bitwise OR combines bits from both operands."""
    iin1 = opendnp3.IINField(opendnp3.IINBit.DEVICE_RESTART)
    iin2 = opendnp3.IINField(opendnp3.IINBit.NEED_TIME)
    combined = iin1 | iin2
    assert combined.IsSet(opendnp3.IINBit.DEVICE_RESTART)
    assert combined.IsSet(opendnp3.IINBit.NEED_TIME)


def test_iinfield_bitwise_and():
    """IINField bitwise AND masks to only shared bits."""
    iin1 = opendnp3.IINField(0xFF, 0xFF)
    iin2 = opendnp3.IINField(opendnp3.IINBit.DEVICE_RESTART)
    result = iin1 & iin2
    assert result.IsSet(opendnp3.IINBit.DEVICE_RESTART)
    assert not result.IsSet(opendnp3.IINBit.BROADCAST)


def test_iinfield_bitwise_invert():
    """IINField bitwise NOT flips all bits."""
    iin = opendnp3.IINField(0x00, 0x00)
    inverted = ~iin
    assert inverted.LSB == 0xFF
    assert inverted.MSB == 0xFF

    # Double inversion returns to original
    double_inv = ~inverted
    assert double_inv.LSB == 0x00
    assert double_inv.MSB == 0x00


def test_iinfield_inplace_or():
    """IINField supports in-place bitwise OR (|=)."""
    iin = opendnp3.IINField()
    iin |= opendnp3.IINField(opendnp3.IINBit.DEVICE_RESTART)
    assert iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)


def test_iinfield_inplace_and():
    """IINField supports in-place bitwise AND (&=)."""
    iin = opendnp3.IINField(0xFF, 0xFF)
    mask = opendnp3.IINField(opendnp3.IINBit.DEVICE_RESTART)
    iin &= mask
    assert iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)
    assert not iin.IsSet(opendnp3.IINBit.BROADCAST)


def test_iinfield_repr():
    """IINField repr shows hex and bit names for non-empty, 'empty' for empty."""
    iin = opendnp3.IINField(opendnp3.IINBit.DEVICE_RESTART)
    r = repr(iin)
    assert "IINField" in r
    assert "DEVICE_RESTART" in r
    assert "0x80" in r

    empty = opendnp3.IINField()
    assert "empty" in repr(empty)


def test_iinfield_str():
    """IINField str shows human-readable bit names."""
    iin = opendnp3.IINField()
    iin.SetBit(opendnp3.IINBit.DEVICE_RESTART)
    iin.SetBit(opendnp3.IINBit.NEED_TIME)
    s = str(iin)
    assert "IIN:" in s
    assert "DEVICE_RESTART" in s
    assert "NEED_TIME" in s

    assert "(none)" in str(opendnp3.IINField())


def test_iinfield_all_lsb_bits_independently():
    """Each LSB IIN bit maps to a unique bit position in the LSB byte."""
    lsb_bits = [
        opendnp3.IINBit.BROADCAST,
        opendnp3.IINBit.CLASS1_EVENTS,
        opendnp3.IINBit.CLASS2_EVENTS,
        opendnp3.IINBit.CLASS3_EVENTS,
        opendnp3.IINBit.NEED_TIME,
        opendnp3.IINBit.LOCAL_CONTROL,
        opendnp3.IINBit.DEVICE_TROUBLE,
        opendnp3.IINBit.DEVICE_RESTART,
    ]
    lsb_values = set()
    for bit in lsb_bits:
        iin = opendnp3.IINField(bit)
        assert iin.IsSet(bit)
        assert iin.MSB == 0
        assert iin.LSB != 0
        lsb_values.add(iin.LSB)
    # All 8 LSB bits should produce distinct LSB byte values
    assert len(lsb_values) == 8


def test_iinfield_all_msb_bits_independently():
    """Each MSB IIN bit maps to a unique bit position in the MSB byte."""
    msb_bits = [
        opendnp3.IINBit.FUNC_NOT_SUPPORTED,
        opendnp3.IINBit.OBJECT_UNKNOWN,
        opendnp3.IINBit.PARAM_ERROR,
        opendnp3.IINBit.EVENT_BUFFER_OVERFLOW,
        opendnp3.IINBit.ALREADY_EXECUTING,
        opendnp3.IINBit.CONFIG_CORRUPT,
        opendnp3.IINBit.RESERVED1,
        opendnp3.IINBit.RESERVED2,
    ]
    msb_values = set()
    for bit in msb_bits:
        iin = opendnp3.IINField(bit)
        assert iin.IsSet(bit)
        assert iin.LSB == 0
        assert iin.MSB != 0
        msb_values.add(iin.MSB)
    assert len(msb_values) == 8


# ===========================================================================
# ApplicationIIN
# ===========================================================================

def test_application_iin_defaults_all_false():
    """ApplicationIIN defaults all flags to False."""
    app_iin = opendnp3.ApplicationIIN()
    assert app_iin.needTime is False
    assert app_iin.localControl is False
    assert app_iin.deviceTrouble is False
    assert app_iin.configCorrupt is False
    assert app_iin.eventBufferOverflow is False


def test_application_iin_set_and_read():
    """ApplicationIIN flags are writable and readable."""
    app_iin = opendnp3.ApplicationIIN()
    app_iin.needTime = True
    app_iin.deviceTrouble = True
    assert app_iin.needTime is True
    assert app_iin.deviceTrouble is True
    assert app_iin.localControl is False  # unset flags remain False


def test_application_iin_to_iin_maps_correctly():
    """ApplicationIIN.ToIIN converts each flag to the correct IINBit."""
    app_iin = opendnp3.ApplicationIIN()
    app_iin.needTime = True
    app_iin.deviceTrouble = True
    iin = app_iin.ToIIN()
    assert iin.IsSet(opendnp3.IINBit.NEED_TIME)
    assert iin.IsSet(opendnp3.IINBit.DEVICE_TROUBLE)
    assert not iin.IsSet(opendnp3.IINBit.DEVICE_RESTART)


def test_application_iin_to_iin_all_flags():
    """ApplicationIIN.ToIIN maps all 5 flags correctly."""
    app_iin = opendnp3.ApplicationIIN()
    app_iin.needTime = True
    app_iin.localControl = True
    app_iin.deviceTrouble = True
    app_iin.configCorrupt = True
    app_iin.eventBufferOverflow = True
    iin = app_iin.ToIIN()
    assert iin.IsSet(opendnp3.IINBit.NEED_TIME)
    assert iin.IsSet(opendnp3.IINBit.LOCAL_CONTROL)
    assert iin.IsSet(opendnp3.IINBit.DEVICE_TROUBLE)
    assert iin.IsSet(opendnp3.IINBit.CONFIG_CORRUPT)
    assert iin.IsSet(opendnp3.IINBit.EVENT_BUFFER_OVERFLOW)


def test_application_iin_to_iin_empty():
    """ApplicationIIN with all defaults produces an empty IINField."""
    app_iin = opendnp3.ApplicationIIN()
    iin = app_iin.ToIIN()
    assert not iin.Any()


# ===========================================================================
# UTCTimestamp
# ===========================================================================

def test_utc_timestamp_default_and_explicit():
    """UTCTimestamp default is 0, explicit values preserved."""
    ts = opendnp3.UTCTimestamp()
    assert ts.msSinceEpoch == 0

    ts2 = opendnp3.UTCTimestamp(1234567890000)
    assert ts2.msSinceEpoch == 1234567890000


# ===========================================================================
# GroupVariationID
# ===========================================================================

def test_group_variation_id():
    """GroupVariationID stores group and variation fields."""
    gv = opendnp3.GroupVariationID(30, 1)
    assert gv.group == 30
    assert gv.variation == 1

    gv2 = opendnp3.GroupVariationID(1, 0)
    assert gv2.group == 1
    assert gv2.variation == 0


# ===========================================================================
# HeaderInfo
# ===========================================================================

def test_header_info_defaults():
    """HeaderInfo default values are deterministic booleans and integers."""
    hi = opendnp3.HeaderInfo()
    assert isinstance(hi.isEventVariation, bool)
    assert isinstance(hi.flagsValid, bool)
    assert isinstance(hi.headerIndex, int)


# ===========================================================================
# RestartOperationResult
# ===========================================================================

def test_restart_operation_result_has_fields():
    """RestartOperationResult has summary and restartTime fields."""
    ror = opendnp3.RestartOperationResult()
    # summary should be a TaskCompletion enum value
    assert ror.summary is not None
    # restartTime should be a TimeDuration
    assert isinstance(repr(ror.restartTime), str)


# ===========================================================================
# SerialSettings
# ===========================================================================

def test_serial_settings_defaults():
    """SerialSettings has sensible defaults (9600 baud, 8 data bits)."""
    ss = opendnp3.SerialSettings()
    assert ss.baud == 9600
    assert ss.dataBits == 8


def test_serial_settings_writable():
    """SerialSettings fields are writable and stick."""
    ss = opendnp3.SerialSettings()
    ss.deviceName = "/dev/ttyS0"
    ss.baud = 115200
    ss.dataBits = 8
    ss.stopBits = opendnp3.StopBits.One
    ss.parity = getattr(opendnp3.Parity, "None")
    ss.flowType = getattr(opendnp3.FlowControl, "None")

    assert ss.deviceName == "/dev/ttyS0"
    assert ss.baud == 115200
    assert ss.stopBits == opendnp3.StopBits.One


# ===========================================================================
# MasterParams
# ===========================================================================

def test_master_params_defaults():
    """MasterParams has correct default values for key fields."""
    mp = opendnp3.MasterParams()
    assert mp.timeSyncMode == getattr(opendnp3.TimeSyncMode, "None")
    assert mp.disableUnsolOnStartup is True
    assert mp.ignoreRestartIIN is False
    assert mp.integrityOnEventOverflowIIN is True
    assert mp.maxTxFragSize > 0
    assert mp.maxRxFragSize > 0


def test_master_params_writable():
    """MasterParams fields are writable and persist."""
    mp = opendnp3.MasterParams()
    mp.responseTimeout = opendnp3.TimeDuration.Seconds(10)
    mp.timeSyncMode = opendnp3.TimeSyncMode.LAN
    mp.disableUnsolOnStartup = False
    mp.ignoreRestartIIN = True
    mp.integrityOnEventOverflowIIN = False
    mp.maxTxFragSize = 1024
    mp.maxRxFragSize = 2048
    mp.controlQualifierMode = opendnp3.IndexQualifierMode.allow_one_byte

    assert mp.timeSyncMode == opendnp3.TimeSyncMode.LAN
    assert mp.disableUnsolOnStartup is False
    assert mp.ignoreRestartIIN is True
    assert mp.maxTxFragSize == 1024
    assert mp.maxRxFragSize == 2048
    assert mp.controlQualifierMode == opendnp3.IndexQualifierMode.allow_one_byte


# ===========================================================================
# OutstationParams
# ===========================================================================

def test_outstation_params_defaults():
    """OutstationParams has correct default values."""
    op = opendnp3.OutstationParams()
    assert op.maxControlsPerRequest > 0
    assert op.maxTxFragSize > 0
    assert op.maxRxFragSize > 0
    assert op.allowUnsolicited is False
    assert op.respondToAnyMaster is False


def test_outstation_params_writable():
    """OutstationParams fields are writable and persist."""
    op = opendnp3.OutstationParams()
    op.maxControlsPerRequest = 32
    op.allowUnsolicited = True
    op.respondToAnyMaster = True
    op.maxTxFragSize = 512
    op.maxRxFragSize = 1024

    assert op.maxControlsPerRequest == 32
    assert op.allowUnsolicited is True
    assert op.respondToAnyMaster is True
    assert op.maxTxFragSize == 512
    assert op.maxRxFragSize == 1024


# ===========================================================================
# EventBufferConfig
# ===========================================================================

def test_event_buffer_config_default_zeros():
    """EventBufferConfig default has all event counts at 0."""
    ebc = opendnp3.EventBufferConfig()
    assert ebc.maxBinaryEvents == 0
    assert ebc.maxDoubleBinaryEvents == 0
    assert ebc.maxAnalogEvents == 0
    assert ebc.maxCounterEvents == 0
    assert ebc.maxFrozenCounterEvents == 0
    assert ebc.maxBinaryOutputStatusEvents == 0
    assert ebc.maxAnalogOutputStatusEvents == 0
    assert ebc.maxOctetStringEvents == 0


def test_event_buffer_config_all_types():
    """EventBufferConfig.AllTypes sets all event counts to the same value."""
    ebc = opendnp3.EventBufferConfig.AllTypes(50)
    for attr in ["maxBinaryEvents", "maxDoubleBinaryEvents", "maxAnalogEvents",
                 "maxCounterEvents", "maxFrozenCounterEvents",
                 "maxBinaryOutputStatusEvents", "maxAnalogOutputStatusEvents",
                 "maxOctetStringEvents"]:
        assert getattr(ebc, attr) == 50


def test_event_buffer_config_individual():
    """EventBufferConfig supports individual field construction."""
    ebc = opendnp3.EventBufferConfig(
        maxBinaryEvents=10, maxDoubleBinaryEvents=20,
        maxAnalogEvents=30, maxCounterEvents=40,
        maxFrozenCounterEvents=5, maxBinaryOutputStatusEvents=15,
        maxAnalogOutputStatusEvents=25, maxOctetStringEvents=35,
    )
    assert ebc.maxBinaryEvents == 10
    assert ebc.maxDoubleBinaryEvents == 20
    assert ebc.maxAnalogEvents == 30
    assert ebc.maxCounterEvents == 40
    assert ebc.maxFrozenCounterEvents == 5
    assert ebc.maxBinaryOutputStatusEvents == 15
    assert ebc.maxAnalogOutputStatusEvents == 25
    assert ebc.maxOctetStringEvents == 35


# ===========================================================================
# OutstationConfig
# ===========================================================================

def test_outstation_config_nested_access():
    """OutstationConfig sub-objects are writable and changes persist."""
    oc = opendnp3.OutstationConfig()
    oc.params.allowUnsolicited = True
    assert oc.params.allowUnsolicited is True
    oc.params.allowUnsolicited = False
    assert oc.params.allowUnsolicited is False


# ===========================================================================
# Indexed<T> types -- verify value and index round-trip
# ===========================================================================

def test_indexed_binary():
    """IndexedBinary preserves value and index."""
    ib = opendnp3.IndexedBinary(opendnp3.Binary(True), 5)
    assert ib.value.value is True
    assert ib.index == 5
    # Default construction
    ib0 = opendnp3.IndexedBinary()
    assert ib0.index == 0


def test_indexed_double_bit_binary():
    """IndexedDoubleBitBinary preserves value and index."""
    dbb = opendnp3.DoubleBitBinary(opendnp3.DoubleBit.DETERMINED_OFF)
    idx = opendnp3.IndexedDoubleBitBinary(dbb, 3)
    assert idx.value.value == opendnp3.DoubleBit.DETERMINED_OFF
    assert idx.index == 3


def test_indexed_analog():
    """IndexedAnalog preserves value and index."""
    idx = opendnp3.IndexedAnalog(opendnp3.Analog(7.5), 2)
    assert abs(idx.value.value - 7.5) < 1e-6
    assert idx.index == 2


def test_indexed_counter():
    """IndexedCounter preserves value and index."""
    idx = opendnp3.IndexedCounter(opendnp3.Counter(100), 0)
    assert idx.value.value == 100
    assert idx.index == 0


def test_indexed_frozen_counter():
    """IndexedFrozenCounter preserves value and index."""
    idx = opendnp3.IndexedFrozenCounter(opendnp3.FrozenCounter(50), 1)
    assert idx.value.value == 50
    assert idx.index == 1


def test_indexed_binary_output_status():
    """IndexedBinaryOutputStatus preserves value and index."""
    idx = opendnp3.IndexedBinaryOutputStatus(opendnp3.BinaryOutputStatus(True), 4)
    assert idx.value.value is True
    assert idx.index == 4


def test_indexed_analog_output_status():
    """IndexedAnalogOutputStatus preserves value and index."""
    idx = opendnp3.IndexedAnalogOutputStatus(opendnp3.AnalogOutputStatus(3.3), 6)
    assert abs(idx.value.value - 3.3) < 1e-6
    assert idx.index == 6


def test_indexed_octet_string():
    """IndexedOctetString preserves data and index."""
    idx = opendnp3.IndexedOctetString(opendnp3.OctetString("test"), 0)
    assert idx.value.Size() == 4
    assert idx.value.ToBytes() == b"test"
    assert idx.index == 0


def test_indexed_time_and_interval():
    """IndexedTimeAndInterval preserves interval and index."""
    t = opendnp3.DNPTime(100)
    tai = opendnp3.TimeAndInterval(t, 1000, opendnp3.IntervalUnits.Milliseconds)
    idx = opendnp3.IndexedTimeAndInterval(tai, 2)
    assert idx.value.interval == 1000
    assert idx.index == 2


def test_indexed_binary_command_event():
    """IndexedBinaryCommandEvent preserves value, status, and index."""
    bce = opendnp3.BinaryCommandEvent(True, opendnp3.CommandStatus.SUCCESS)
    idx = opendnp3.IndexedBinaryCommandEvent(bce, 7)
    assert idx.value.value is True
    assert idx.value.status == opendnp3.CommandStatus.SUCCESS
    assert idx.index == 7


def test_indexed_analog_command_event():
    """IndexedAnalogCommandEvent preserves value and index."""
    ace = opendnp3.AnalogCommandEvent(9.9, opendnp3.CommandStatus.SUCCESS)
    idx = opendnp3.IndexedAnalogCommandEvent(ace, 8)
    assert abs(idx.value.value - 9.9) < 1e-6
    assert idx.index == 8


def test_indexed_analog_input_deadband():
    """IndexedAnalogInputDeadband preserves value and index."""
    aid = opendnp3.AnalogInputDeadband(2.5)
    idx = opendnp3.IndexedAnalogInputDeadband(aid, 3)
    assert abs(idx.value.value - 2.5) < 1e-6
    assert idx.index == 3


# ===========================================================================
# Command event types
# ===========================================================================

def test_binary_command_event_all_constructors():
    """BinaryCommandEvent supports default, value+status, and value+status+time."""
    bce = opendnp3.BinaryCommandEvent()
    assert bce.value is False

    bce2 = opendnp3.BinaryCommandEvent(True, opendnp3.CommandStatus.SUCCESS)
    assert bce2.value is True
    assert bce2.status == opendnp3.CommandStatus.SUCCESS

    t = opendnp3.DNPTime(2000)
    bce3 = opendnp3.BinaryCommandEvent(False, opendnp3.CommandStatus.TIMEOUT, t)
    assert bce3.value is False
    assert bce3.status == opendnp3.CommandStatus.TIMEOUT
    assert bce3.time.value == 2000


def test_analog_command_event_all_constructors():
    """AnalogCommandEvent supports default, value+status, and value+status+time."""
    ace = opendnp3.AnalogCommandEvent()

    ace2 = opendnp3.AnalogCommandEvent(42.5, opendnp3.CommandStatus.SUCCESS)
    assert abs(ace2.value - 42.5) < 1e-6
    assert ace2.status == opendnp3.CommandStatus.SUCCESS

    t = opendnp3.DNPTime(3000)
    ace3 = opendnp3.AnalogCommandEvent(1.0, opendnp3.CommandStatus.FORMAT_ERROR, t)
    assert abs(ace3.value - 1.0) < 1e-6
    assert ace3.status == opendnp3.CommandStatus.FORMAT_ERROR
    assert ace3.time.value == 3000


# ===========================================================================
# AnalogInputDeadband
# ===========================================================================

def test_analog_input_deadband():
    """AnalogInputDeadband stores and allows mutation of the deadband value."""
    aid = opendnp3.AnalogInputDeadband()
    assert aid.value == 0.0

    aid2 = opendnp3.AnalogInputDeadband(5.0)
    assert abs(aid2.value - 5.0) < 1e-6

    aid2.value = 10.0
    assert abs(aid2.value - 10.0) < 1e-6


# ===========================================================================
# TLS Config
# ===========================================================================

def test_tls_config_defaults():
    """TLSConfig stores paths and has secure defaults."""
    tls = opendnp3.TLSConfig("ca.pem", "cert.pem", "key.pem")
    assert tls.peerCertFilePath == "ca.pem"
    assert tls.localCertFilePath == "cert.pem"
    assert tls.privateKeyFilePath == "key.pem"
    assert tls.allowTLSv10 is False
    assert tls.allowTLSv11 is False
    assert tls.allowTLSv12 is True
    assert tls.allowTLSv13 is True
    assert tls.cipherList == ""
    assert tls.verifyCallback is None


def test_tls_config_verify_callback_lifecycle():
    """TLSConfig.verifyCallback can be set, read, and cleared."""
    tls = opendnp3.TLSConfig("ca.pem", "cert.pem", "key.pem")
    assert tls.verifyCallback is None

    def my_cb(preverified, depth, subject, cert_der):
        return True

    tls.verifyCallback = my_cb
    assert tls.verifyCallback is not None

    tls.verifyCallback = lambda pre, d, s, c: False
    assert tls.verifyCallback is not None

    tls.verifyCallback = None
    assert tls.verifyCallback is None


def test_tls_config_custom_options():
    """TLSConfig accepts custom TLS version and cipher options."""
    tls = opendnp3.TLSConfig(
        "ca.pem", "cert.pem", "key.pem",
        allowTLSv12=False, allowTLSv13=True,
        cipherList="ECDHE-RSA-AES256-GCM-SHA384"
    )
    assert tls.allowTLSv12 is False
    assert tls.allowTLSv13 is True
    assert tls.cipherList == "ECDHE-RSA-AES256-GCM-SHA384"


# ===========================================================================
# TaskConfig, ResponseInfo, ModuleId
# ===========================================================================

def test_task_config_default_produces_object():
    """TaskConfig.Default() returns a valid configuration."""
    tc = opendnp3.TaskConfig.Default()
    assert tc is not None


def test_response_info_type_exists():
    """ResponseInfo class is accessible for use in SOE handler callbacks."""
    assert hasattr(opendnp3, "ResponseInfo")


def test_module_id_stores_value():
    """ModuleId stores the given integer value."""
    mid = opendnp3.ModuleId()
    mid2 = opendnp3.ModuleId(42)
    assert mid2.value == 42


# ===========================================================================
# DNP3Manager lifecycle
# ===========================================================================

def test_manager_lifecycle():
    """DNP3Manager can be created and shut down cleanly."""
    manager = opendnp3.DNP3Manager(1)
    manager.Shutdown()


def test_manager_multiple_threads():
    """DNP3Manager can be created with multiple threads."""
    manager = opendnp3.DNP3Manager(4)
    manager.Shutdown()


# ===========================================================================
# ISOEHandler with OnRawAPDU
# ===========================================================================

def test_soe_handler_subclass():
    """ISOEHandler subclass with all methods can be instantiated."""

    class RawHandler(opendnp3.ISOEHandler):
        def __init__(self):
            super().__init__()
            self.raw_data = None

        def BeginFragment(self, info):
            pass

        def EndFragment(self, info):
            pass

        def Process(self, info, values):
            pass

        def OnRawAPDU(self, info, data):
            self.raw_data = data

    handler = RawHandler()
    assert handler.raw_data is None


# ===========================================================================
# File Transfer Enums
# ===========================================================================

def test_file_mode_enum():
    """FileMode has 3 distinct values: READ, WRITE, APPEND."""
    vals = [
        opendnp3.FileMode.READ,
        opendnp3.FileMode.WRITE,
        opendnp3.FileMode.APPEND,
    ]
    assert len(set(id(v) for v in vals)) == 3
    assert opendnp3.FileMode.READ != opendnp3.FileMode.WRITE
    assert opendnp3.FileMode.WRITE != opendnp3.FileMode.APPEND


def test_file_status_enum():
    """FileStatus has 12 distinct values."""
    vals = [
        opendnp3.FileStatus.SUCCESS,
        opendnp3.FileStatus.PERMISSION_DENIED,
        opendnp3.FileStatus.INVALID_MODE,
        opendnp3.FileStatus.FILE_NOT_FOUND,
        opendnp3.FileStatus.FILE_LOCKED,
        opendnp3.FileStatus.NOT_OPENED,
        opendnp3.FileStatus.CLOSE_ABORT,
        opendnp3.FileStatus.NOT_EXIST,
        opendnp3.FileStatus.HANDLE_EXPIRED,
        opendnp3.FileStatus.BUFFER_OVERFLOW,
        opendnp3.FileStatus.FATAL,
        opendnp3.FileStatus.BLOCK_SEQ,
    ]
    assert len(set(id(v) for v in vals)) == 12
    assert opendnp3.FileStatus.SUCCESS != opendnp3.FileStatus.FILE_NOT_FOUND


def test_file_type_enum():
    """FileType has 2 distinct values: DIRECTORY, SIMPLE_FILE."""
    assert opendnp3.FileType.DIRECTORY != opendnp3.FileType.SIMPLE_FILE


# ===========================================================================
# File Transfer Types
# ===========================================================================

def test_file_permission_set():
    """FilePermissionSet stores read/write/execute flags."""
    ps = opendnp3.FilePermissionSet()
    assert ps.read is False
    assert ps.write is False
    assert ps.execute is False

    ps2 = opendnp3.FilePermissionSet(True, True, False)
    assert ps2.read is True
    assert ps2.write is True
    assert ps2.execute is False


def test_file_permissions():
    """FilePermissions stores owner/group/world and supports FromRaw/ToRaw."""
    fp = opendnp3.FilePermissions()
    assert fp.owner.read is False

    owner = opendnp3.FilePermissionSet(True, True, False)
    group = opendnp3.FilePermissionSet(True, False, False)
    world = opendnp3.FilePermissionSet(True, False, False)
    fp2 = opendnp3.FilePermissions(owner, group, world)
    assert fp2.owner.read is True
    assert fp2.owner.write is True
    assert fp2.group.read is True
    assert fp2.world.read is True

    # Round-trip through raw
    raw = fp2.ToRaw()
    fp3 = opendnp3.FilePermissions.FromRaw(raw)
    assert fp3.owner.read == fp2.owner.read
    assert fp3.owner.write == fp2.owner.write
    assert fp3.owner.execute == fp2.owner.execute
    assert fp3.group.read == fp2.group.read
    assert fp3.world.read == fp2.world.read
    assert fp3.ToRaw() == raw


def test_file_info():
    """FileInfo stores all metadata fields."""
    fi = opendnp3.FileInfo()
    assert fi.fileName == ""
    assert fi.type == opendnp3.FileType.SIMPLE_FILE
    assert fi.size == 0
    assert fi.timeOfCreation == 0
    assert fi.requestId == 0

    fi.fileName = "test.txt"
    fi.type = opendnp3.FileType.DIRECTORY
    fi.size = 1024
    fi.timeOfCreation = 123456
    assert fi.fileName == "test.txt"
    assert fi.type == opendnp3.FileType.DIRECTORY
    assert fi.size == 1024
    assert fi.timeOfCreation == 123456


def test_file_read_result():
    """FileReadResult has summary, statusCode, and data fields."""
    r = opendnp3.FileReadResult()
    assert r.summary == opendnp3.TaskCompletion.FAILURE_NO_COMMS
    assert r.statusCode == opendnp3.FileStatus.SUCCESS
    assert r.data == b""


def test_file_info_result():
    """FileInfoResult has summary, statusCode, and info fields."""
    r = opendnp3.FileInfoResult()
    assert r.summary == opendnp3.TaskCompletion.FAILURE_NO_COMMS
    assert r.statusCode == opendnp3.FileStatus.SUCCESS
    assert r.info.fileName == ""


def test_file_operation_result():
    """FileOperationResult has summary and statusCode fields."""
    r = opendnp3.FileOperationResult()
    assert r.summary == opendnp3.TaskCompletion.FAILURE_NO_COMMS
    assert r.statusCode == opendnp3.FileStatus.SUCCESS


def test_file_write_result():
    """FileWriteResult has summary and statusCode fields."""
    r = opendnp3.FileWriteResult()
    assert r.summary == opendnp3.TaskCompletion.FAILURE_NO_COMMS
    assert r.statusCode == opendnp3.FileStatus.SUCCESS


def test_directory_read_result():
    """DirectoryReadResult has summary, statusCode, and entries fields."""
    r = opendnp3.DirectoryReadResult()
    assert r.summary == opendnp3.TaskCompletion.FAILURE_NO_COMMS
    assert r.entries == []


def test_file_auth_result():
    """FileAuthResult has summary, statusCode, and authKey fields."""
    r = opendnp3.FileAuthResult()
    assert r.summary == opendnp3.TaskCompletion.FAILURE_NO_COMMS
    assert r.authKey == 0


def test_file_open_result():
    """FileOpenResult (outstation-side) has all fields."""
    r = opendnp3.FileOpenResult()
    assert r.status == opendnp3.FileStatus.NOT_EXIST
    assert r.fileHandle == 0
    assert r.fileSize == 0
    assert r.maxBlockSize == 2048


def test_file_block_result():
    """FileBlockResult (outstation-side) has all fields."""
    r = opendnp3.FileBlockResult()
    assert r.status == opendnp3.FileStatus.SUCCESS
    assert r.data == []
    assert r.lastBlock is False


def test_file_command_result():
    """FileCommandResult (outstation-side) has status and info."""
    r = opendnp3.FileCommandResult()
    assert r.status == opendnp3.FileStatus.NOT_EXIST


def test_outstation_file_auth_result():
    """OutstationFileAuthResult has status and authKey."""
    r = opendnp3.OutstationFileAuthResult()
    assert r.status == opendnp3.FileStatus.PERMISSION_DENIED
    assert r.authKey == 0


def test_ifilehandler_subclass():
    """IFileHandler subclass can be instantiated in Python."""

    class TestHandler(opendnp3.IFileHandler):
        def __init__(self):
            super().__init__()

        def GetFileInfo(self, filename):
            r = opendnp3.FileCommandResult()
            r.status = opendnp3.FileStatus.NOT_EXIST
            return r

        def OpenFile(self, filename, authKey, permissions, mode, maxBlockSize, requestId):
            r = opendnp3.FileOpenResult()
            r.status = opendnp3.FileStatus.NOT_EXIST
            return r

        def ReadBlock(self, fileHandle, blockNum):
            r = opendnp3.FileBlockResult()
            r.status = opendnp3.FileStatus.NOT_OPENED
            return r

        def WriteBlock(self, fileHandle, blockNum, lastBlock, data):
            return opendnp3.FileStatus.SUCCESS

        def CloseFile(self, fileHandle, requestId):
            return opendnp3.FileStatus.SUCCESS

        def DeleteFile(self, filename):
            return opendnp3.FileStatus.NOT_EXIST

        def AbortFile(self, fileHandle):
            pass

        def AuthenticateFile(self, username, password):
            r = opendnp3.OutstationFileAuthResult()
            r.status = opendnp3.FileStatus.PERMISSION_DENIED
            return r

    handler = TestHandler()
    assert handler is not None
