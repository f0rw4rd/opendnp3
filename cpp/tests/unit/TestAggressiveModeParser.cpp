/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "secauth/AggressiveModeParser.h"
#include "utils/BufferHelpers.h"

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "AggressiveModeParser - " name

TEST_CASE(SUITE("empty buffer is not aggressive mode"))
{
    ser4cpp::rseq_t empty;
    auto result = AggressiveModeParser::IsAggressiveMode(empty, nullptr);
    REQUIRE(result.result == ParseResult::OK);
    REQUIRE_FALSE(result.isAggMode);
}

TEST_CASE(SUITE("non-g120v3 header is not aggressive mode"))
{
    // group=1, variation=2 (not 120/3), qualifier=0x00 (start-stop 1 byte), start=0, stop=0
    HexSequence hex("01 02 00 00 00");
    auto result = AggressiveModeParser::IsAggressiveMode(hex.ToRSeq(), nullptr);
    REQUIRE(result.result == ParseResult::OK);
    REQUIRE_FALSE(result.isAggMode);
}

TEST_CASE(SUITE("g120v3 with correct data is aggressive mode"))
{
    // group=120 (0x78), var=3, qualifier=0x07 (UINT8_CNT), count=1
    // Then 6 bytes of Group120Var3: challengeSeqNum(4) + userNum(2)
    HexSequence hex("78 03 07 01 01 00 00 00 05 00");
    auto result = AggressiveModeParser::IsAggressiveMode(hex.ToRSeq(), nullptr);
    REQUIRE(result.result == ParseResult::OK);
    REQUIRE(result.isAggMode);
    REQUIRE(result.request.challengeSeqNum == 1);
    REQUIRE(result.request.userNum == 5);
}

TEST_CASE(SUITE("g120v3 with bad qualifier rejects"))
{
    // qualifier=0xFF instead of 0x07
    HexSequence hex("78 03 FF 01 01 00 00 00 05 00");
    auto result = AggressiveModeParser::IsAggressiveMode(hex.ToRSeq(), nullptr);
    REQUIRE(result.result == ParseResult::INVALID_OBJECT_QUALIFIER);
    REQUIRE_FALSE(result.isAggMode);
}

TEST_CASE(SUITE("g120v3 with count != 1 rejects"))
{
    // count=2 instead of 1
    HexSequence hex("78 03 07 02 01 00 00 00 05 00");
    auto result = AggressiveModeParser::IsAggressiveMode(hex.ToRSeq(), nullptr);
    REQUIRE(result.result == ParseResult::NOT_ON_WHITELIST);
    REQUIRE_FALSE(result.isAggMode);
}

TEST_CASE(SUITE("g120v3 with insufficient data for object"))
{
    // correct header but only 4 of 6 bytes for the object
    HexSequence hex("78 03 07 01 01 00 00 00");
    auto result = AggressiveModeParser::IsAggressiveMode(hex.ToRSeq(), nullptr);
    REQUIRE(result.result == ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS);
    REQUIRE_FALSE(result.isAggMode);
}

TEST_CASE(SUITE("too short for object header"))
{
    HexSequence hex("78");
    auto result = AggressiveModeParser::IsAggressiveMode(hex.ToRSeq(), nullptr);
    REQUIRE(result.result == ParseResult::NOT_ENOUGH_DATA_FOR_HEADER);
    REQUIRE_FALSE(result.isAggMode);
}
