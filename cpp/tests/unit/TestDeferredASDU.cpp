/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "secauth/DeferredASDU.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "DeferredASDU - " name

TEST_CASE(SUITE("default is not set"))
{
    DeferredASDU asdu;
    REQUIRE_FALSE(asdu.IsSet());
}

TEST_CASE(SUITE("default Get returns empty"))
{
    DeferredASDU asdu;
    auto data = asdu.Get();
    REQUIRE(data.length() == 0);
}

TEST_CASE(SUITE("Set stores data and IsSet returns true"))
{
    DeferredASDU asdu;
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    ser4cpp::rseq_t input(payload, 5);
    REQUIRE(asdu.Set(input));
    REQUIRE(asdu.IsSet());
}

TEST_CASE(SUITE("Get returns stored data"))
{
    DeferredASDU asdu;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC};
    ser4cpp::rseq_t input(payload, 3);
    asdu.Set(input);

    auto data = asdu.Get();
    REQUIRE(data.length() == 3);
    REQUIRE(data[0] == 0xAA);
    REQUIRE(data[1] == 0xBB);
    REQUIRE(data[2] == 0xCC);
}

TEST_CASE(SUITE("Clear resets state"))
{
    DeferredASDU asdu;
    uint8_t payload[] = {0x01, 0x02};
    ser4cpp::rseq_t input(payload, 2);
    asdu.Set(input);
    REQUIRE(asdu.IsSet());

    asdu.Clear();
    REQUIRE_FALSE(asdu.IsSet());
    auto data = asdu.Get();
    REQUIRE(data.length() == 0);
}

TEST_CASE(SUITE("Set can be called multiple times"))
{
    DeferredASDU asdu;

    uint8_t first[] = {0x01, 0x02, 0x03};
    asdu.Set(ser4cpp::rseq_t(first, 3));
    REQUIRE(asdu.Get().length() == 3);

    uint8_t second[] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
    asdu.Set(ser4cpp::rseq_t(second, 5));
    auto data = asdu.Get();
    REQUIRE(data.length() == 5);
    REQUIRE(data[0] == 0x0A);
}

TEST_CASE(SUITE("Set with empty data"))
{
    DeferredASDU asdu;
    ser4cpp::rseq_t empty;
    REQUIRE(asdu.Set(empty));
    REQUIRE(asdu.IsSet());
    REQUIRE(asdu.Get().length() == 0);
}

TEST_CASE(SUITE("data is copied not referenced"))
{
    DeferredASDU asdu;
    uint8_t payload[] = {0xDE, 0xAD};
    ser4cpp::rseq_t input(payload, 2);
    asdu.Set(input);

    // modify original
    payload[0] = 0x00;
    payload[1] = 0x00;

    // stored data should be unchanged
    auto data = asdu.Get();
    REQUIRE(data[0] == 0xDE);
    REQUIRE(data[1] == 0xAD);
}
