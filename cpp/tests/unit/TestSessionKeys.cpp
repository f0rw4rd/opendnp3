/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "secauth/SessionKeys.h"
#include "secauth/SessionStore.h"

#include <opendnp3/gen/KeyStatus.h>
#include <opendnp3/util/TimeDuration.h>
#include <opendnp3/util/Timestamp.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "SessionKeys - " name

TEST_CASE(SUITE("default constructor creates empty keys"))
{
    SessionKeys keys;
    auto view = keys.GetView();
    REQUIRE_FALSE(view.IsValid());
    REQUIRE(view.controlKey.length() == 0);
    REQUIRE(view.monitorKey.length() == 0);
}

TEST_CASE(SUITE("SetKeys then GetView returns same data"))
{
    uint8_t controlData[]
        = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t monitorData[]
        = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0};
    ser4cpp::rseq_t controlKey(controlData, 16);
    ser4cpp::rseq_t monitorKey(monitorData, 16);
    SessionKeysView input(controlKey, monitorKey);

    SessionKeys keys;
    keys.SetKeys(input);

    auto view = keys.GetView();
    REQUIRE(view.IsValid());
    REQUIRE(view.controlKey.length() == 16);
    REQUIRE(view.monitorKey.length() == 16);
    REQUIRE(view.controlKey.equals(controlKey));
    REQUIRE(view.monitorKey.equals(monitorKey));
}

TEST_CASE(SUITE("SessionKeysView default is invalid"))
{
    SessionKeysView view;
    REQUIRE_FALSE(view.IsValid());
}

TEST_CASE(SUITE("SessionKeysView with data is valid"))
{
    uint8_t a[] = {1, 2, 3};
    uint8_t b[] = {4, 5, 6};
    SessionKeysView view(ser4cpp::rseq_t(a, 3), ser4cpp::rseq_t(b, 3));
    REQUIRE(view.IsValid());
}

// --- SessionEntry tests ---

TEST_CASE(SUITE("SessionEntry: newly created has NOT_INIT status"))
{
    SessionEntry entry(TimeDuration::Minutes(15), 1000);
    Timestamp now;
    REQUIRE(entry.GetKeyStatus(now) == KeyStatus::NOT_INIT);
}

TEST_CASE(SUITE("SessionEntry: after SetKeys status is OK"))
{
    SessionEntry entry(TimeDuration::Minutes(15), 1000);
    uint8_t cd[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t md[] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0};
    SessionKeysView keys(ser4cpp::rseq_t(cd, 16), ser4cpp::rseq_t(md, 16));

    Timestamp now;
    entry.SetKeys(keys, now);
    REQUIRE(entry.GetKeyStatus(now) == KeyStatus::OK);
}

TEST_CASE(SUITE("SessionEntry: IncrementAuthCount works until maxAuthCount"))
{
    SessionEntry entry(TimeDuration::Minutes(15), 3);
    uint8_t cd[16] = {};
    uint8_t md[16] = {};
    SessionKeysView keys(ser4cpp::rseq_t(cd, 16), ser4cpp::rseq_t(md, 16));

    Timestamp now;
    entry.SetKeys(keys, now);

    REQUIRE(entry.IncrementAuthCount(now) == KeyStatus::OK);
    REQUIRE(entry.IncrementAuthCount(now) == KeyStatus::OK);
    // third increment hits maxAuthCount=3
    REQUIRE(entry.IncrementAuthCount(now) == KeyStatus::COMM_FAIL);
}

TEST_CASE(SUITE("SessionEntry: expired key returns COMM_FAIL"))
{
    SessionEntry entry(TimeDuration::Milliseconds(100), 1000);
    uint8_t cd[16] = {};
    uint8_t md[16] = {};
    SessionKeysView keys(ser4cpp::rseq_t(cd, 16), ser4cpp::rseq_t(md, 16));

    Timestamp now;
    entry.SetKeys(keys, now);
    REQUIRE(entry.GetKeyStatus(now) == KeyStatus::OK);

    // advance past expiration
    Timestamp future = now + TimeDuration::Milliseconds(200);
    REQUIRE(entry.GetKeyStatus(future) == KeyStatus::COMM_FAIL);
}

TEST_CASE(SUITE("SessionEntry: TryGetKeyView returns keys when OK"))
{
    SessionEntry entry(TimeDuration::Minutes(15), 1000);
    uint8_t cd[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t md[] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0};
    SessionKeysView keys(ser4cpp::rseq_t(cd, 16), ser4cpp::rseq_t(md, 16));

    Timestamp now;
    entry.SetKeys(keys, now);

    SessionKeysView output;
    auto status = entry.TryGetKeyView(output, now);
    REQUIRE(status == KeyStatus::OK);
    REQUIRE(output.IsValid());
    REQUIRE(output.controlKey.length() == 16);
}

// --- SessionStore tests ---

TEST_CASE(SUITE("SessionStore: TryGetKeyView for unknown user returns NOT_INIT"))
{
    SessionStore store(TimeDuration::Minutes(15), 1000);
    SessionKeysView view;
    Timestamp now;
    auto status = store.TryGetKeyView(42, view, now);
    REQUIRE(status == KeyStatus::NOT_INIT);
}

TEST_CASE(SUITE("SessionStore: SetKeys and retrieve for known user"))
{
    SessionStore store(TimeDuration::Minutes(15), 1000);
    uint8_t cd[16] = {};
    uint8_t md[16] = {};
    SessionKeysView keys(ser4cpp::rseq_t(cd, 16), ser4cpp::rseq_t(md, 16));

    Timestamp now;
    store.SetKeys(1, keys, now);

    SessionKeysView view;
    auto status = store.TryGetKeyView(1, view, now);
    REQUIRE(status == KeyStatus::OK);
    REQUIRE(view.IsValid());
}

TEST_CASE(SUITE("SessionStore: GetKeyStatus for unknown user"))
{
    SessionStore store(TimeDuration::Minutes(15), 1000);
    Timestamp now;
    REQUIRE(store.GetKeyStatus(99, now) == KeyStatus::NOT_INIT);
}

TEST_CASE(SUITE("SessionStore: IncrementAuthCount for unknown user"))
{
    SessionStore store(TimeDuration::Minutes(15), 1000);
    Timestamp now;
    REQUIRE(store.IncrementAuthCount(99, now) == KeyStatus::NOT_INIT);
}
