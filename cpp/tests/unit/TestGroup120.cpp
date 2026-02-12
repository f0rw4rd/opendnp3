/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Modified 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/BufferHelpers.h"

#include <ser4cpp/container/Buffer.h>

#include <catch.hpp>
#include <gen/objects/Group120.h>

using namespace opendnp3;
using namespace ser4cpp;

#define SUITE(name) "Group120TestSuite - " name

// ============================================================
// Group120Var1 - Authentication Challenge
// ============================================================

TEST_CASE(SUITE("g120v1 - empty buffer returns false"))
{
    Group120Var1 obj;
    rseq_t empty;
    REQUIRE_FALSE(obj.Read(empty));
}

TEST_CASE(SUITE("g120v1 - parse challenge with data"))
{
    HexSequence hex("01 00 00 00 07 00 05 01 DE AD BE EF");
    Group120Var1 obj;
    REQUIRE(obj.Read(hex.ToRSeq()));
    REQUIRE(obj.challengeSeqNum == 1);
    REQUIRE(obj.userNum == 7);
    REQUIRE(obj.hmacAlgo == HMACType::HMAC_SHA1_TRUNC_8);
    REQUIRE(obj.challengeReason == ChallengeReason::CRITICAL);
    REQUIRE(obj.challengeData.length() == 4);
}

TEST_CASE(SUITE("g120v1 - parse min size with empty challenge data"))
{
    HexSequence hex("01 00 00 00 07 00 05 01");
    Group120Var1 obj;
    REQUIRE(obj.Read(hex.ToRSeq()));
    REQUIRE(obj.challengeSeqNum == 1);
    REQUIRE(obj.userNum == 7);
    REQUIRE(obj.challengeData.length() == 0);
}

TEST_CASE(SUITE("g120v1 - one less than min size returns false"))
{
    HexSequence hex("01 00 00 00 07 00 05");
    Group120Var1 obj;
    REQUIRE_FALSE(obj.Read(hex.ToRSeq()));
}

TEST_CASE(SUITE("g120v1 - write challenge"))
{
    HexSequence challengeData("DE AD BE EF AB BA");
    Group120Var1 obj(9, 3, HMACType::HMAC_SHA256_TRUNC_16, ChallengeReason::CRITICAL, challengeData.ToRSeq());

    REQUIRE(obj.Size() == 14);

    Buffer output(64);
    auto dest = output.as_wslice();
    REQUIRE(obj.Write(dest));

    HexSequence expected("09 00 00 00 03 00 04 01 DE AD BE EF AB BA");
    rseq_t written = output.as_rslice().take(14);
    REQUIRE(written.equals(expected.ToRSeq()));
}

TEST_CASE(SUITE("g120v1 - write insufficient space returns false"))
{
    HexSequence challengeData("DE AD BE EF AB BA");
    Group120Var1 obj(9, 3, HMACType::HMAC_SHA256_TRUNC_16, ChallengeReason::CRITICAL, challengeData.ToRSeq());

    Buffer output(5);
    auto dest = output.as_wslice();
    REQUIRE_FALSE(obj.Write(dest));
}

// ============================================================
// Group120Var2 - Authentication Reply
// ============================================================

TEST_CASE(SUITE("g120v2 - empty buffer returns false"))
{
    Group120Var2 obj;
    rseq_t empty;
    REQUIRE_FALSE(obj.Read(empty));
}

TEST_CASE(SUITE("g120v2 - parse reply with hmac data"))
{
    HexSequence hex("04 00 00 00 09 01 AB BA");
    Group120Var2 obj;
    REQUIRE(obj.Read(hex.ToRSeq()));
    REQUIRE(obj.challengeSeqNum == 4);
    REQUIRE(obj.userNum == 265);
    REQUIRE(obj.hmacValue.length() == 2);
}

TEST_CASE(SUITE("g120v2 - parse min size with empty hmac"))
{
    HexSequence hex("04 00 00 00 09 01");
    Group120Var2 obj;
    REQUIRE(obj.Read(hex.ToRSeq()));
    REQUIRE(obj.challengeSeqNum == 4);
    REQUIRE(obj.userNum == 265);
    REQUIRE(obj.hmacValue.length() == 0);
}

TEST_CASE(SUITE("g120v2 - one less than min size returns false"))
{
    HexSequence hex("04 00 00 00 09");
    Group120Var2 obj;
    REQUIRE_FALSE(obj.Read(hex.ToRSeq()));
}

// ============================================================
// Group120Var5 - Session Key Status
// ============================================================

TEST_CASE(SUITE("g120v5 - empty buffer returns false"))
{
    Group120Var5 obj;
    rseq_t empty;
    REQUIRE_FALSE(obj.Read(empty));
}

TEST_CASE(SUITE("g120v5 - parse key status with empty challenge and hmac"))
{
    HexSequence hex("01 00 00 00 07 00 02 01 04 00 00");
    Group120Var5 obj;
    REQUIRE(obj.Read(hex.ToRSeq()));
    REQUIRE(obj.keyChangeSeqNum == 1);
    REQUIRE(obj.userNum == 7);
    REQUIRE(obj.keyWrapAlgo == KeyWrapAlgorithm::AES_256);
    REQUIRE(obj.keyStatus == KeyStatus::OK);
    REQUIRE(obj.hmacAlgo == HMACType::HMAC_SHA256_TRUNC_16);
    REQUIRE(obj.challengeData.length() == 0);
    REQUIRE(obj.hmacValue.length() == 0);
}

TEST_CASE(SUITE("g120v5 - parse key status with challenge and hmac data"))
{
    HexSequence hex("01 00 00 00 07 00 02 01 04 03 00 DE AD BE EF");
    Group120Var5 obj;
    REQUIRE(obj.Read(hex.ToRSeq()));
    REQUIRE(obj.keyChangeSeqNum == 1);
    REQUIRE(obj.userNum == 7);
    REQUIRE(obj.keyWrapAlgo == KeyWrapAlgorithm::AES_256);
    REQUIRE(obj.keyStatus == KeyStatus::OK);
    REQUIRE(obj.hmacAlgo == HMACType::HMAC_SHA256_TRUNC_16);
    REQUIRE(obj.challengeData.length() == 3);
    REQUIRE(obj.hmacValue.length() == 1);
}

TEST_CASE(SUITE("g120v5 - one less than min returns false (missing second byte of challengeDataLength)"))
{
    HexSequence hex("01 00 00 00 07 00 02 01 04 00");
    Group120Var5 obj;
    REQUIRE_FALSE(obj.Read(hex.ToRSeq()));
}

TEST_CASE(SUITE("g120v5 - challenge len=1 but no data returns false"))
{
    HexSequence hex("01 00 00 00 07 00 02 01 04 01 00");
    Group120Var5 obj;
    REQUIRE_FALSE(obj.Read(hex.ToRSeq()));
}

TEST_CASE(SUITE("g120v5 - write key status"))
{
    HexSequence challenge("DE AD");
    HexSequence hmac("BE EF");
    Group120Var5 obj(8, 3, KeyWrapAlgorithm::AES_256, KeyStatus::OK, HMACType::HMAC_SHA1_TRUNC_8, challenge.ToRSeq(),
                     hmac.ToRSeq());

    REQUIRE(obj.Size() == 15);

    Buffer output(64);
    auto dest = output.as_wslice();
    REQUIRE(obj.Write(dest));

    HexSequence expected("08 00 00 00 03 00 02 01 05 02 00 DE AD BE EF");
    rseq_t written = output.as_rslice().take(15);
    REQUIRE(written.equals(expected.ToRSeq()));
}

TEST_CASE(SUITE("g120v5 - write insufficient space returns false"))
{
    HexSequence challenge("DE AD");
    HexSequence hmac("BE EF");
    Group120Var5 obj(8, 3, KeyWrapAlgorithm::AES_256, KeyStatus::OK, HMACType::HMAC_SHA1_TRUNC_8, challenge.ToRSeq(),
                     hmac.ToRSeq());

    Buffer output(5);
    auto dest = output.as_wslice();
    REQUIRE_FALSE(obj.Write(dest));
}
