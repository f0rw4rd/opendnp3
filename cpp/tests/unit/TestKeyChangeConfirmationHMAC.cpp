/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "crypto/SHA256HMAC.h"
#include "secauth/KeyChangeConfirmationHMAC.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "KeyChangeConfirmationHMAC - " name

TEST_CASE(SUITE("Compute produces non-empty output"))
{
    SHA256HMAC algo;
    KeyChangeConfirmationHMAC hmac;

    uint8_t keyData[32];
    for (int i = 0; i < 32; ++i)
        keyData[i] = static_cast<uint8_t>(i);
    ser4cpp::rseq_t key(keyData, 32);

    uint8_t ocData[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t mcData[] = {0x05, 0x06, 0x07, 0x08};
    ser4cpp::rseq_t outstationChallenge(ocData, 4);
    ser4cpp::rseq_t masterChallenge(mcData, 4);

    std::error_code ec;
    auto result = hmac.Compute(algo, key, 1, 7, outstationChallenge, masterChallenge, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(result.length() > 0);
}

TEST_CASE(SUITE("same inputs produce same output"))
{
    SHA256HMAC algo;
    KeyChangeConfirmationHMAC hmac1;
    KeyChangeConfirmationHMAC hmac2;

    uint8_t keyData[32];
    for (int i = 0; i < 32; ++i)
        keyData[i] = static_cast<uint8_t>(i + 0x10);
    ser4cpp::rseq_t key(keyData, 32);

    uint8_t ocData[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t mcData[] = {0x11, 0x22, 0x33, 0x44};
    ser4cpp::rseq_t outstationChallenge(ocData, 4);
    ser4cpp::rseq_t masterChallenge(mcData, 4);

    std::error_code ec1, ec2;
    auto result1 = hmac1.Compute(algo, key, 5, 3, outstationChallenge, masterChallenge, ec1);
    auto result2 = hmac2.Compute(algo, key, 5, 3, outstationChallenge, masterChallenge, ec2);

    REQUIRE_FALSE(ec1);
    REQUIRE_FALSE(ec2);
    REQUIRE(result1.length() == result2.length());
    REQUIRE(result1.equals(result2));
}

TEST_CASE(SUITE("different key produces different output"))
{
    SHA256HMAC algo;
    KeyChangeConfirmationHMAC hmac1;
    KeyChangeConfirmationHMAC hmac2;

    uint8_t keyData1[32];
    uint8_t keyData2[32];
    for (int i = 0; i < 32; ++i)
    {
        keyData1[i] = static_cast<uint8_t>(i);
        keyData2[i] = static_cast<uint8_t>(i + 0x80);
    }
    ser4cpp::rseq_t key1(keyData1, 32);
    ser4cpp::rseq_t key2(keyData2, 32);

    uint8_t ocData[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t mcData[] = {0x05, 0x06, 0x07, 0x08};
    ser4cpp::rseq_t outstationChallenge(ocData, 4);
    ser4cpp::rseq_t masterChallenge(mcData, 4);

    std::error_code ec1, ec2;
    auto result1 = hmac1.Compute(algo, key1, 1, 7, outstationChallenge, masterChallenge, ec1);
    auto result2 = hmac2.Compute(algo, key2, 1, 7, outstationChallenge, masterChallenge, ec2);

    REQUIRE_FALSE(ec1);
    REQUIRE_FALSE(ec2);
    REQUIRE(result1.length() > 0);
    REQUIRE(result2.length() > 0);
    REQUIRE_FALSE(result1.equals(result2));
}

TEST_CASE(SUITE("different userNum produces different output"))
{
    SHA256HMAC algo;
    KeyChangeConfirmationHMAC hmac1;
    KeyChangeConfirmationHMAC hmac2;

    uint8_t keyData[32];
    for (int i = 0; i < 32; ++i)
        keyData[i] = static_cast<uint8_t>(i);
    ser4cpp::rseq_t key(keyData, 32);

    uint8_t ocData[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t mcData[] = {0x05, 0x06, 0x07, 0x08};
    ser4cpp::rseq_t outstationChallenge(ocData, 4);
    ser4cpp::rseq_t masterChallenge(mcData, 4);

    std::error_code ec1, ec2;
    auto result1 = hmac1.Compute(algo, key, 1, 7, outstationChallenge, masterChallenge, ec1);
    auto result2 = hmac2.Compute(algo, key, 1, 8, outstationChallenge, masterChallenge, ec2);

    REQUIRE_FALSE(ec1);
    REQUIRE_FALSE(ec2);
    REQUIRE_FALSE(result1.equals(result2));
}

TEST_CASE(SUITE("different KSQ produces different output"))
{
    SHA256HMAC algo;
    KeyChangeConfirmationHMAC hmac1;
    KeyChangeConfirmationHMAC hmac2;

    uint8_t keyData[32];
    for (int i = 0; i < 32; ++i)
        keyData[i] = static_cast<uint8_t>(i);
    ser4cpp::rseq_t key(keyData, 32);

    uint8_t ocData[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t mcData[] = {0x05, 0x06, 0x07, 0x08};
    ser4cpp::rseq_t outstationChallenge(ocData, 4);
    ser4cpp::rseq_t masterChallenge(mcData, 4);

    std::error_code ec1, ec2;
    auto result1 = hmac1.Compute(algo, key, 1, 7, outstationChallenge, masterChallenge, ec1);
    auto result2 = hmac2.Compute(algo, key, 2, 7, outstationChallenge, masterChallenge, ec2);

    REQUIRE_FALSE(ec1);
    REQUIRE_FALSE(ec2);
    REQUIRE_FALSE(result1.equals(result2));
}
