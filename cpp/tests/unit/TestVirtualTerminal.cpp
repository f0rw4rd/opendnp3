/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include "utils/BufferHelpers.h"

#include <opendnp3/gen/GroupVariation.h>
#include <opendnp3/logging/LogLevels.h>

#include <ser4cpp/util/HexConversions.h>

#include "dnp3mocks/MockAPDUHeaderHandler.h"
#include "dnp3mocks/MockLogHandler.h"

#include <app/parsing/APDUParser.h>
#include <catch.hpp>

using namespace opendnp3;
using namespace ser4cpp;

#define SUITE(name) "VirtualTerminalTestSuite - " name

TEST_CASE(SUITE("GroupVariation enum values for VT"))
{
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group112Var0) == 0x7000);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group113Var0) == 0x7100);
}

TEST_CASE(SUITE("GroupVariation string mappings"))
{
    REQUIRE(std::string("Group112Var0") == GroupVariationSpec::to_string(GroupVariation::Group112Var0));
    REQUIRE(std::string("Group113Var0") == GroupVariationSpec::to_string(GroupVariation::Group113Var0));
}

TEST_CASE(SUITE("GroupVariation human strings"))
{
    REQUIRE(std::string("Virtual Terminal Output Block - Sized by variation")
            == GroupVariationSpec::to_human_string(GroupVariation::Group112Var0));
    REQUIRE(std::string("Virtual Terminal Event Data - Sized by variation")
            == GroupVariationSpec::to_human_string(GroupVariation::Group113Var0));
}

TEST_CASE(SUITE("Parse Group 112 range - VT Output Block"))
{
    // Group 112 with variation=3 (3 bytes per object), range qualifier 0x00 (1-byte start/stop)
    // Range: start=0, stop=1 (2 objects, each 3 bytes)
    const std::string hex = "70 03 00"  // group 112, var 3, qualifier 0x00 (UINT8_START_STOP)
                            "00 01"     // range: start=0, stop=1
                            "41 42 43"  // object 0: "ABC"
                            "44 45 46"; // object 1: "DEF"

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::OK);
    REQUIRE(mock.records.size() == 1);
    REQUIRE(mock.rangedOctets.size() == 2);

    // Verify the first octet string
    REQUIRE(mock.rangedOctets[0].index == 0);
    REQUIRE(mock.rangedOctets[0].value.ToBuffer().length == 3);

    // Verify the second octet string
    REQUIRE(mock.rangedOctets[1].index == 1);
    REQUIRE(mock.rangedOctets[1].value.ToBuffer().length == 3);
}

TEST_CASE(SUITE("Parse Group 113 count+index - VT Event Data"))
{
    // Group 113 with variation=4 (4 bytes per object), count+index qualifier 0x17 (1-byte count, 1-byte index)
    // Count=1, index=5, data=4 bytes
    const std::string hex = "71 04 17"     // group 113, var 4, qualifier 0x17 (UINT8_CNT_UINT8_INDEX)
                            "01"           // count = 1
                            "05"           // index = 5
                            "48 65 6C 6C"; // data: "Hell"

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::OK);
    REQUIRE(mock.records.size() == 1);
    REQUIRE(mock.indexPrefixedOctets.size() == 1);
    REQUIRE(mock.indexPrefixedOctets[0].index == 5);
    REQUIRE(mock.indexPrefixedOctets[0].value.ToBuffer().length == 4);
}

TEST_CASE(SUITE("Parse Group 112 variation 0 in response is rejected"))
{
    // Group 112, variation 0 should only be used in requests (no data expected)
    // When used with range qualifier and content, it should be rejected
    const std::string hex = "70 00 00" // group 112, var 0, qualifier 0x00 (UINT8_START_STOP)
                            "00 00";   // range: start=0, stop=0

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    // Variation 0 with contents should fail
    REQUIRE(result == ParseResult::INVALID_OBJECT);
}

TEST_CASE(SUITE("Parse Group 113 variation 0 in response is rejected"))
{
    // Group 113, variation 0 should only be used in requests
    const std::string hex = "71 00 17" // group 113, var 0, qualifier 0x17 (UINT8_CNT_UINT8_INDEX)
                            "01"       // count = 1
                            "00";      // index = 0 (but no data since var=0)

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::INVALID_OBJECT);
}

TEST_CASE(SUITE("Parse Group 112 multiple objects"))
{
    // Group 112 variation 2 with 3 objects (range 0-2)
    const std::string hex = "70 02 00" // group 112, var 2, qualifier 0x00
                            "00 02"    // range: start=0, stop=2
                            "41 42"    // object 0: "AB"
                            "43 44"    // object 1: "CD"
                            "45 46";   // object 2: "EF"

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::OK);
    REQUIRE(mock.rangedOctets.size() == 3);
    REQUIRE(mock.rangedOctets[0].index == 0);
    REQUIRE(mock.rangedOctets[1].index == 1);
    REQUIRE(mock.rangedOctets[2].index == 2);
}
