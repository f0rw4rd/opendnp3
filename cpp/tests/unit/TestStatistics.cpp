/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "secauth/outstation/Statistics.h"

#include <opendnp3/gen/SecurityStatIndex.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "Statistics - " name

TEST_CASE(SUITE("all 18 counters start at 0"))
{
    Statistics stats;
    REQUIRE(stats.GetValue(SecurityStatIndex::UNEXPECTED_MESSAGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::AUTHORIZATION_FAILURES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::AUTHENTICATION_FAILURES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::REPLY_TIMEOUTS) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::REKEYS_DUE_TO_AUTH_FAILURE) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::TOTAL_MESSAGES_TX) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::TOTAL_MESSAGES_RX) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::CRITICAL_MESSAGES_TX) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::CRITICAL_MESSAGES_RX) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::DISCARDED_MESSAGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::ERROR_MESSAGES_TX) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::ERROR_MESSAGES_RX) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::SUCCESSFUL_AUTHS) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::SESSION_KEY_CHANGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::FAILED_SESSION_KEY_CHANGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::UPDATE_KEY_CHANGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::FAILED_UPDATE_KEY_CHANGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::REKEYS_DUE_TO_RESTART) == 0);
}

TEST_CASE(SUITE("Increment returns new value"))
{
    Statistics stats;
    auto val = stats.Increment(SecurityStatIndex::UNEXPECTED_MESSAGES);
    REQUIRE(val == 1);
}

TEST_CASE(SUITE("multiple increments accumulate"))
{
    Statistics stats;
    stats.Increment(SecurityStatIndex::TOTAL_MESSAGES_TX);
    stats.Increment(SecurityStatIndex::TOTAL_MESSAGES_TX);
    stats.Increment(SecurityStatIndex::TOTAL_MESSAGES_TX);
    REQUIRE(stats.GetValue(SecurityStatIndex::TOTAL_MESSAGES_TX) == 3);
}

TEST_CASE(SUITE("GetValue returns current count"))
{
    Statistics stats;
    REQUIRE(stats.GetValue(SecurityStatIndex::AUTHENTICATION_FAILURES) == 0);
    stats.Increment(SecurityStatIndex::AUTHENTICATION_FAILURES);
    REQUIRE(stats.GetValue(SecurityStatIndex::AUTHENTICATION_FAILURES) == 1);
    stats.Increment(SecurityStatIndex::AUTHENTICATION_FAILURES);
    REQUIRE(stats.GetValue(SecurityStatIndex::AUTHENTICATION_FAILURES) == 2);
}

TEST_CASE(SUITE("incrementing one stat does not affect others"))
{
    Statistics stats;
    stats.Increment(SecurityStatIndex::SUCCESSFUL_AUTHS);
    stats.Increment(SecurityStatIndex::SUCCESSFUL_AUTHS);
    REQUIRE(stats.GetValue(SecurityStatIndex::SUCCESSFUL_AUTHS) == 2);
    REQUIRE(stats.GetValue(SecurityStatIndex::SESSION_KEY_CHANGES) == 0);
    REQUIRE(stats.GetValue(SecurityStatIndex::UNEXPECTED_MESSAGES) == 0);
}
