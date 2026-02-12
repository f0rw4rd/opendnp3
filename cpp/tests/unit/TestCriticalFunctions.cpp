/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <opendnp3/secauth/CriticalFunctions.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "CriticalFunctions - " name

TEST_CASE(SUITE("AuthOptional: WRITE is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::WRITE));
}

TEST_CASE(SUITE("AuthOptional: SELECT is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::SELECT));
}

TEST_CASE(SUITE("AuthOptional: OPERATE is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::OPERATE));
}

TEST_CASE(SUITE("AuthOptional: DIRECT_OPERATE is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::DIRECT_OPERATE));
}

TEST_CASE(SUITE("AuthOptional: DIRECT_OPERATE_NR is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::DIRECT_OPERATE_NR));
}

TEST_CASE(SUITE("AuthOptional: COLD_RESTART is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::COLD_RESTART));
}

TEST_CASE(SUITE("AuthOptional: WARM_RESTART is always critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE(cf.IsCritical(FunctionCode::WARM_RESTART));
}

TEST_CASE(SUITE("AuthOptional: READ is NOT critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE_FALSE(cf.IsCritical(FunctionCode::READ));
}

TEST_CASE(SUITE("AuthOptional: CONFIRM is NOT critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE_FALSE(cf.IsCritical(FunctionCode::CONFIRM));
}

TEST_CASE(SUITE("AuthOptional: IMMED_FREEZE is NOT critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE_FALSE(cf.IsCritical(FunctionCode::IMMED_FREEZE));
}

TEST_CASE(SUITE("AuthOptional: DELAY_MEASURE is NOT critical"))
{
    auto cf = CriticalFunctions::AuthOptional();
    REQUIRE_FALSE(cf.IsCritical(FunctionCode::DELAY_MEASURE));
}

TEST_CASE(SUITE("AuthEverything: READ IS critical"))
{
    auto cf = CriticalFunctions::AuthEverything();
    REQUIRE(cf.IsCritical(FunctionCode::READ));
}

TEST_CASE(SUITE("AuthEverything: CONFIRM IS critical"))
{
    auto cf = CriticalFunctions::AuthEverything();
    REQUIRE(cf.IsCritical(FunctionCode::CONFIRM));
}

TEST_CASE(SUITE("AuthEverything: IMMED_FREEZE IS critical"))
{
    auto cf = CriticalFunctions::AuthEverything();
    REQUIRE(cf.IsCritical(FunctionCode::IMMED_FREEZE));
}

TEST_CASE(SUITE("AuthEverything: DELAY_MEASURE IS critical"))
{
    auto cf = CriticalFunctions::AuthEverything();
    REQUIRE(cf.IsCritical(FunctionCode::DELAY_MEASURE));
}

TEST_CASE(SUITE("AuthEverything: WRITE is still critical"))
{
    auto cf = CriticalFunctions::AuthEverything();
    REQUIRE(cf.IsCritical(FunctionCode::WRITE));
}

TEST_CASE(SUITE("AuthEverything: all optional codes are critical"))
{
    auto cf = CriticalFunctions::AuthEverything();
    REQUIRE(cf.IsCritical(FunctionCode::ASSIGN_CLASS));
    REQUIRE(cf.IsCritical(FunctionCode::RESPONSE));
    REQUIRE(cf.IsCritical(FunctionCode::UNSOLICITED_RESPONSE));
    REQUIRE(cf.IsCritical(FunctionCode::FREEZE_CLEAR));
    REQUIRE(cf.IsCritical(FunctionCode::FREEZE_AT_TIME));
    REQUIRE(cf.IsCritical(FunctionCode::INITIALIZE_DATA));
}
