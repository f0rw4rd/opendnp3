/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <opendnp3/secauth/StatThresholds.h>

#include <catch.hpp>

#include <limits>

using namespace opendnp3;

#define SUITE(name) "StatThresholds - " name

TEST_CASE(SUITE("default thresholds are non-zero"))
{
    StatThresholds thresholds;
    for (uint16_t i = 0; i < NUM_SECURITY_STATS; ++i)
    {
        REQUIRE(thresholds.GetDeadband(i) > 0);
    }
}

TEST_CASE(SUITE("default thresholds are 1"))
{
    StatThresholds thresholds;
    for (uint16_t i = 0; i < NUM_SECURITY_STATS; ++i)
    {
        REQUIRE(thresholds.GetDeadband(i) == 1);
    }
}

TEST_CASE(SUITE("out of range index returns UINT32_MAX"))
{
    StatThresholds thresholds;
    REQUIRE(thresholds.GetDeadband(NUM_SECURITY_STATS) == std::numeric_limits<uint32_t>::max());
    REQUIRE(thresholds.GetDeadband(255) == std::numeric_limits<uint32_t>::max());
}

TEST_CASE(SUITE("Set and GetDeadband round-trip"))
{
    StatThresholds thresholds;
    thresholds.Set(SecurityStatIndex::UNEXPECTED_MESSAGES, 42);
    REQUIRE(thresholds.GetDeadband(0) == 42);
}

TEST_CASE(SUITE("individual threshold override works"))
{
    StatThresholds thresholds;
    thresholds.Set(SecurityStatIndex::AUTHENTICATION_FAILURES, 100);
    // the overridden one
    REQUIRE(thresholds.GetDeadband(static_cast<uint16_t>(SecurityStatIndex::AUTHENTICATION_FAILURES)) == 100);
    // others untouched
    REQUIRE(thresholds.GetDeadband(static_cast<uint16_t>(SecurityStatIndex::UNEXPECTED_MESSAGES)) == 1);
    REQUIRE(thresholds.GetDeadband(static_cast<uint16_t>(SecurityStatIndex::TOTAL_MESSAGES_TX)) == 1);
}

TEST_CASE(SUITE("Set multiple thresholds"))
{
    StatThresholds thresholds;
    thresholds.Set(SecurityStatIndex::SUCCESSFUL_AUTHS, 10);
    thresholds.Set(SecurityStatIndex::SESSION_KEY_CHANGES, 20);
    thresholds.Set(SecurityStatIndex::UPDATE_KEY_CHANGES, 30);
    REQUIRE(thresholds.GetDeadband(static_cast<uint16_t>(SecurityStatIndex::SUCCESSFUL_AUTHS)) == 10);
    REQUIRE(thresholds.GetDeadband(static_cast<uint16_t>(SecurityStatIndex::SESSION_KEY_CHANGES)) == 20);
    REQUIRE(thresholds.GetDeadband(static_cast<uint16_t>(SecurityStatIndex::UPDATE_KEY_CHANGES)) == 30);
}
