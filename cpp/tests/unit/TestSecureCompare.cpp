/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <opendnp3/crypto/SecureCompare.h>

#include <ser4cpp/container/SequenceTypes.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "SecureCompare - " name

TEST_CASE(SUITE("equal buffers return true"))
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    ser4cpp::rseq_t lhs(data, 4);
    ser4cpp::rseq_t rhs(data, 4);
    REQUIRE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("different buffers return false"))
{
    uint8_t a[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t b[] = {0x01, 0x02, 0x03, 0x05};
    ser4cpp::rseq_t lhs(a, 4);
    ser4cpp::rseq_t rhs(b, 4);
    REQUIRE_FALSE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("different lengths return false"))
{
    uint8_t a[] = {0x01, 0x02, 0x03};
    uint8_t b[] = {0x01, 0x02, 0x03, 0x04};
    ser4cpp::rseq_t lhs(a, 3);
    ser4cpp::rseq_t rhs(b, 4);
    REQUIRE_FALSE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("empty buffers return true"))
{
    ser4cpp::rseq_t lhs;
    ser4cpp::rseq_t rhs;
    REQUIRE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("single byte difference detected"))
{
    uint8_t a[] = {0xAA};
    uint8_t b[] = {0xAB};
    ser4cpp::rseq_t lhs(a, 1);
    ser4cpp::rseq_t rhs(b, 1);
    REQUIRE_FALSE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("single byte match"))
{
    uint8_t a[] = {0xFF};
    uint8_t b[] = {0xFF};
    ser4cpp::rseq_t lhs(a, 1);
    ser4cpp::rseq_t rhs(b, 1);
    REQUIRE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("longer equal buffers"))
{
    uint8_t data[32];
    for (int i = 0; i < 32; ++i)
        data[i] = static_cast<uint8_t>(i);
    uint8_t copy[32];
    for (int i = 0; i < 32; ++i)
        copy[i] = static_cast<uint8_t>(i);
    ser4cpp::rseq_t lhs(data, 32);
    ser4cpp::rseq_t rhs(copy, 32);
    REQUIRE(SecureEquals(lhs, rhs));
}

TEST_CASE(SUITE("difference in last byte detected"))
{
    uint8_t a[16];
    uint8_t b[16];
    for (int i = 0; i < 16; ++i)
    {
        a[i] = static_cast<uint8_t>(i);
        b[i] = static_cast<uint8_t>(i);
    }
    b[15] = 0xFF;
    ser4cpp::rseq_t lhs(a, 16);
    ser4cpp::rseq_t rhs(b, 16);
    REQUIRE_FALSE(SecureEquals(lhs, rhs));
}
