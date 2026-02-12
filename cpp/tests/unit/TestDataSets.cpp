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
#include <gen/objects/Group85.h>
#include <gen/objects/Group86.h>
#include <gen/objects/Group87.h>
#include <gen/objects/Group88.h>

using namespace opendnp3;
using namespace ser4cpp;

#define SUITE(name) "DataSetTestSuite - " name

TEST_CASE(SUITE("GroupVariation enum values for Data Sets"))
{
    // Verify the enum values match the DNP3 wire format
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group85Var1) == 0x5501);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group86Var0) == 0x5600);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group86Var1) == 0x5601);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group86Var2) == 0x5602);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group86Var3) == 0x5603);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group87Var1) == 0x5701);
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group88Var1) == 0x5801);
}

TEST_CASE(SUITE("GroupVariation string mappings"))
{
    REQUIRE(std::string("Group85Var1") == GroupVariationSpec::to_string(GroupVariation::Group85Var1));
    REQUIRE(std::string("Group86Var1") == GroupVariationSpec::to_string(GroupVariation::Group86Var1));
    REQUIRE(std::string("Group86Var2") == GroupVariationSpec::to_string(GroupVariation::Group86Var2));
    REQUIRE(std::string("Group86Var3") == GroupVariationSpec::to_string(GroupVariation::Group86Var3));
    REQUIRE(std::string("Group87Var1") == GroupVariationSpec::to_string(GroupVariation::Group87Var1));
    REQUIRE(std::string("Group88Var1") == GroupVariationSpec::to_string(GroupVariation::Group88Var1));
}

TEST_CASE(SUITE("GroupVariation from_type mapping"))
{
    REQUIRE(GroupVariation::Group85Var1 == GroupVariationSpec::from_type(0x5501));
    REQUIRE(GroupVariation::Group86Var1 == GroupVariationSpec::from_type(0x5601));
    REQUIRE(GroupVariation::Group86Var2 == GroupVariationSpec::from_type(0x5602));
    REQUIRE(GroupVariation::Group86Var3 == GroupVariationSpec::from_type(0x5603));
    REQUIRE(GroupVariation::Group87Var1 == GroupVariationSpec::from_type(0x5701));
    REQUIRE(GroupVariation::Group88Var1 == GroupVariationSpec::from_type(0x5801));
}

TEST_CASE(SUITE("GroupVariation from_string mapping"))
{
    REQUIRE(GroupVariation::Group85Var1 == GroupVariationSpec::from_string("Group85Var1"));
    REQUIRE(GroupVariation::Group86Var1 == GroupVariationSpec::from_string("Group86Var1"));
    REQUIRE(GroupVariation::Group87Var1 == GroupVariationSpec::from_string("Group87Var1"));
    REQUIRE(GroupVariation::Group88Var1 == GroupVariationSpec::from_string("Group88Var1"));
}

TEST_CASE(SUITE("GroupVariation human strings"))
{
    REQUIRE(std::string("Data Set Prototype - With UUID")
            == GroupVariationSpec::to_human_string(GroupVariation::Group85Var1));
    REQUIRE(std::string("Data Set Descriptor - Contents")
            == GroupVariationSpec::to_human_string(GroupVariation::Group86Var1));
    REQUIRE(std::string("Data Set Descriptor - Characteristics")
            == GroupVariationSpec::to_human_string(GroupVariation::Group86Var2));
    REQUIRE(std::string("Data Set Descriptor - Point Index Attributes")
            == GroupVariationSpec::to_human_string(GroupVariation::Group86Var3));
    REQUIRE(std::string("Data Set Present Value") == GroupVariationSpec::to_human_string(GroupVariation::Group87Var1));
    REQUIRE(std::string("Data Set Snapshot - Event")
            == GroupVariationSpec::to_human_string(GroupVariation::Group88Var1));
}

TEST_CASE(SUITE("Group85Var1 Read with UUID"))
{
    Group85Var1 obj;

    // 16-byte UUID + 3 bytes of element descriptor data
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
                      0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0xAA, 0xBB, 0xCC};
    ser4cpp::rseq_t buffer(data, sizeof(data));

    REQUIRE(obj.Read(buffer));
    REQUIRE(obj.uuid[0] == 0x01);
    REQUIRE(obj.uuid[15] == 0x10);
    REQUIRE(obj.elements.size() == 3);
    REQUIRE(obj.elements[0] == 0xAA);
}

TEST_CASE(SUITE("Group85Var1 Read too short"))
{
    Group85Var1 obj;

    uint8_t data[] = {0x01, 0x02, 0x03};
    ser4cpp::rseq_t buffer(data, sizeof(data));

    REQUIRE_FALSE(obj.Read(buffer));
}

TEST_CASE(SUITE("Group86Var1 Read"))
{
    Group86Var1 obj;

    uint8_t data[] = {0x01, 0x05, 0x48, 0x65, 0x6C, 0x6C, 0x6F};
    ser4cpp::rseq_t buffer(data, sizeof(data));

    REQUIRE(obj.Read(buffer));
    REQUIRE(obj.data.size() == 7);
}

TEST_CASE(SUITE("Group87Var1 Read"))
{
    Group87Var1 obj;

    uint8_t data[] = {0x05, 0x48, 0x65, 0x6C, 0x6C, 0x6F};
    ser4cpp::rseq_t buffer(data, sizeof(data));

    REQUIRE(obj.Read(buffer));
    REQUIRE(obj.data.size() == 6);
}

TEST_CASE(SUITE("Group88Var1 Read with timestamp"))
{
    Group88Var1 obj;

    // 6-byte timestamp (value 0x060504030201 = little-endian) + 2 bytes data
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xAA, 0xBB};
    ser4cpp::rseq_t buffer(data, sizeof(data));

    REQUIRE(obj.Read(buffer));

    // Verify timestamp
    uint64_t expected = 0;
    for (int j = 0; j < 6; ++j)
    {
        expected |= static_cast<uint64_t>(data[j]) << (j * 8);
    }
    REQUIRE(obj.timestamp == expected);
    REQUIRE(obj.data.size() == 2);
    REQUIRE(obj.data[0] == 0xAA);
    REQUIRE(obj.data[1] == 0xBB);
}

TEST_CASE(SUITE("Group88Var1 Read too short"))
{
    Group88Var1 obj;

    uint8_t data[] = {0x01, 0x02, 0x03};
    ser4cpp::rseq_t buffer(data, sizeof(data));

    REQUIRE_FALSE(obj.Read(buffer));
}

TEST_CASE(SUITE("Parse Data Set free-format object - Group86Var1"))
{
    // Build an APDU fragment with a free-format Data Set descriptor:
    // Object header: group=86, var=1, qualifier=0x5B (free-format)
    // count=1, length=4 (2 bytes LE), data=4 bytes
    const std::string hex = "56 01 5B"     // group 86, var 1, qualifier 0x5B
                            "01"           // count = 1
                            "04 00"        // length = 4 (little-endian)
                            "01 05 41 42"; // descriptor data: type=1, maxlen=5, name='AB'

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::OK);
    // The data set is delivered as OctetString through the handler
    REQUIRE(mock.rangedOctets.size() == 1);
}

TEST_CASE(SUITE("Parse Data Set free-format object - Group88Var1"))
{
    // Build an APDU fragment with a free-format Data Set snapshot:
    // Object header: group=88, var=1, qualifier=0x5B (free-format)
    // count=1, length=8, data=6-byte timestamp + 2 bytes value data
    const std::string hex = "58 01 5B"          // group 88, var 1, qualifier 0x5B
                            "01"                // count = 1
                            "08 00"             // length = 8 (little-endian)
                            "01 02 03 04 05 06" // timestamp
                            "AA BB";            // value data

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::OK);
    REQUIRE(mock.rangedOctets.size() == 1);
}
