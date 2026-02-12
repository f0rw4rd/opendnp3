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

#include <opendnp3/app/DeviceAttributes.h>
#include <opendnp3/gen/GroupVariation.h>
#include <opendnp3/logging/LogLevels.h>

#include <ser4cpp/util/HexConversions.h>

#include "dnp3mocks/MockAPDUHeaderHandler.h"
#include "dnp3mocks/MockLogHandler.h"

#include <app/parsing/APDUParser.h>
#include <catch.hpp>

using namespace opendnp3;
using namespace ser4cpp;

#define SUITE(name) "DeviceAttrUnicodeTestSuite - " name

TEST_CASE(SUITE("DeviceAttrType UNICODE enum value"))
{
    REQUIRE(static_cast<uint8_t>(DeviceAttrType::UNICODE) == 0x08);
}

TEST_CASE(SUITE("Group50Var0 enum value"))
{
    REQUIRE(static_cast<uint16_t>(GroupVariation::Group50Var0) == 0x3200);
}

TEST_CASE(SUITE("Group50Var0 string mapping"))
{
    REQUIRE(std::string("Group50Var0") == GroupVariationSpec::to_string(GroupVariation::Group50Var0));
}

TEST_CASE(SUITE("Group50Var0 human string"))
{
    REQUIRE(std::string("Time and Date - Default Variation")
            == GroupVariationSpec::to_human_string(GroupVariation::Group50Var0));
}

TEST_CASE(SUITE("Group50Var0 from_type"))
{
    REQUIRE(GroupVariation::Group50Var0 == GroupVariationSpec::from_type(0x3200));
}

TEST_CASE(SUITE("Group50Var0 from_string"))
{
    REQUIRE(GroupVariation::Group50Var0 == GroupVariationSpec::from_string("Group50Var0"));
}

TEST_CASE(SUITE("Parse Group0 device attribute with Unicode type code"))
{
    // Group 0, variation 210 (example), range qualifier 0x00
    // Range: start=210, stop=210 (1 object)
    // Object: type_code=8 (Unicode), length=4, payload="Test" (as Unicode-designated)
    const std::string hex = "00 D2 00"     // group 0, var 210, qualifier 0x00 (UINT8_START_STOP)
                            "D2 D2"        // range: start=210, stop=210
                            "08"           // type_code = 8 (Unicode)
                            "04"           // payload length = 4
                            "54 65 73 74"; // "Test"

    HexSequence buffer(hex);
    MockApduHeaderHandler mock;
    MockLogHandler log;

    auto result = APDUParser::Parse(buffer.ToRSeq(), mock, &log.logger);
    REQUIRE(result == ParseResult::OK);
}
