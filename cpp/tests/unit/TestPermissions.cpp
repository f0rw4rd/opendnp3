/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "secauth/outstation/Permissions.h"
#include "secauth/outstation/RoleBasedPermissions.h"

#include <opendnp3/gen/UserRole.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "Permissions - " name

TEST_CASE(SUITE("default permissions deny everything"))
{
    Permissions p;
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::READ));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::OPERATE));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::DIRECT_OPERATE));
}

TEST_CASE(SUITE("AllowNothing denies everything"))
{
    auto p = Permissions::AllowNothing();
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::READ));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
}

TEST_CASE(SUITE("AllowAll permits everything"))
{
    auto p = Permissions::AllowAll();
    REQUIRE(p.IsAllowed(FunctionCode::READ));
    REQUIRE(p.IsAllowed(FunctionCode::WRITE));
    REQUIRE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE(p.IsAllowed(FunctionCode::OPERATE));
    REQUIRE(p.IsAllowed(FunctionCode::COLD_RESTART));
    REQUIRE(p.IsAllowed(FunctionCode::WARM_RESTART));
}

TEST_CASE(SUITE("Allow enables specific function code"))
{
    Permissions p;
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
    p.Allow(FunctionCode::WRITE);
    REQUIRE(p.IsAllowed(FunctionCode::WRITE));
    // other codes still denied
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::READ));
}

TEST_CASE(SUITE("IsAllowed returns true only for allowed codes"))
{
    Permissions p;
    p.Allow(FunctionCode::READ);
    p.Allow(FunctionCode::SELECT);
    REQUIRE(p.IsAllowed(FunctionCode::READ));
    REQUIRE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::OPERATE));
}

TEST_CASE(SUITE("Allowed variadic factory creates correct bitfield"))
{
    auto p = Permissions::Allowed(FunctionCode::READ, FunctionCode::WRITE, FunctionCode::OPERATE);
    REQUIRE(p.IsAllowed(FunctionCode::READ));
    REQUIRE(p.IsAllowed(FunctionCode::WRITE));
    REQUIRE(p.IsAllowed(FunctionCode::OPERATE));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::DIRECT_OPERATE));
}

TEST_CASE(SUITE("operator| combines permissions"))
{
    auto a = Permissions::Allowed(FunctionCode::READ);
    auto b = Permissions::Allowed(FunctionCode::WRITE);
    auto combined = a | b;
    REQUIRE(combined.IsAllowed(FunctionCode::READ));
    REQUIRE(combined.IsAllowed(FunctionCode::WRITE));
    REQUIRE_FALSE(combined.IsAllowed(FunctionCode::SELECT));
}

TEST_CASE(SUITE("RoleBasedPermissions: VIEWER can read but not write"))
{
    auto p = RoleBasedPermissions::From(UserRole::VIEWER);
    REQUIRE(p.IsAllowed(FunctionCode::READ));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::OPERATE));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::DIRECT_OPERATE));
}

TEST_CASE(SUITE("RoleBasedPermissions: OPERATOR can read and operate"))
{
    auto p = RoleBasedPermissions::From(UserRole::OPERATOR);
    REQUIRE(p.IsAllowed(FunctionCode::READ));
    REQUIRE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE(p.IsAllowed(FunctionCode::OPERATE));
    REQUIRE(p.IsAllowed(FunctionCode::DIRECT_OPERATE));
    // operator cannot write
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
}

TEST_CASE(SUITE("RoleBasedPermissions: SINGLE_USER can do everything"))
{
    auto p = RoleBasedPermissions::From(UserRole::SINGLE_USER);
    REQUIRE(p.IsAllowed(FunctionCode::READ));
    REQUIRE(p.IsAllowed(FunctionCode::WRITE));
    REQUIRE(p.IsAllowed(FunctionCode::SELECT));
    REQUIRE(p.IsAllowed(FunctionCode::OPERATE));
    REQUIRE(p.IsAllowed(FunctionCode::DIRECT_OPERATE));
    REQUIRE(p.IsAllowed(FunctionCode::COLD_RESTART));
    REQUIRE(p.IsAllowed(FunctionCode::WARM_RESTART));
}

TEST_CASE(SUITE("RoleBasedPermissions: UNDEFINED role gets nothing"))
{
    auto p = RoleBasedPermissions::From(UserRole::UNDEFINED);
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::READ));
    REQUIRE_FALSE(p.IsAllowed(FunctionCode::WRITE));
}
