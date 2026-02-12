/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "secauth/outstation/OutstationUserDatabase.h"
#include "secauth/outstation/OutstationUserInfo.h"
#include "secauth/outstation/Permissions.h"
#include "secauth/outstation/RoleBasedPermissions.h"

#include <opendnp3/gen/UserRole.h>
#include <opendnp3/secauth/UpdateKey.h>

#include <catch.hpp>

using namespace opendnp3;

#define SUITE(name) "OutstationUserDatabase - " name

static OutstationUserInfo MakeUser(uint16_t num, const std::string& name, UserRole role)
{
    return OutstationUserInfo(num, name, role, 365, UpdateKey(0xAA, KeyWrapAlgorithm::AES_128),
                              RoleBasedPermissions::From(role));
}

TEST_CASE(SUITE("empty database: FindByUserNum returns false"))
{
    OutstationUserDatabase db;
    OutstationUserInfo info;
    REQUIRE_FALSE(db.FindByUserNum(1, info));
}

TEST_CASE(SUITE("empty database: FindByUserName returns false"))
{
    OutstationUserDatabase db;
    OutstationUserInfo info;
    REQUIRE_FALSE(db.FindByUserName("alice", info));
}

TEST_CASE(SUITE("AddOrUpdate then FindByUserNum returns true"))
{
    OutstationUserDatabase db;
    auto user = MakeUser(1, "alice", UserRole::OPERATOR);
    db.AddOrUpdate(user);

    OutstationUserInfo info;
    REQUIRE(db.FindByUserNum(1, info));
    REQUIRE(info.userName == "alice");
    REQUIRE(info.userNum == 1);
    REQUIRE(info.role == UserRole::OPERATOR);
}

TEST_CASE(SUITE("AddOrUpdate then FindByUserName returns true"))
{
    OutstationUserDatabase db;
    auto user = MakeUser(1, "bob", UserRole::VIEWER);
    db.AddOrUpdate(user);

    OutstationUserInfo info;
    REQUIRE(db.FindByUserName("bob", info));
    REQUIRE(info.userNum == 1);
}

TEST_CASE(SUITE("GetUpdateKey for existing user succeeds"))
{
    OutstationUserDatabase db;
    auto user = MakeUser(5, "charlie", UserRole::SINGLE_USER);
    db.AddOrUpdate(user);

    UpdateKey key;
    REQUIRE(db.GetUpdateKey(5, key));
    REQUIRE(key.IsValid());
    REQUIRE(key.GetView().algorithm == KeyWrapAlgorithm::AES_128);
}

TEST_CASE(SUITE("GetUpdateKey for nonexistent user fails"))
{
    OutstationUserDatabase db;
    UpdateKey key;
    REQUIRE_FALSE(db.GetUpdateKey(99, key));
}

TEST_CASE(SUITE("Delete removes user"))
{
    OutstationUserDatabase db;
    db.AddOrUpdate(MakeUser(1, "alice", UserRole::OPERATOR));

    OutstationUserInfo info;
    REQUIRE(db.FindByUserNum(1, info));

    REQUIRE(db.Delete("alice"));
    REQUIRE_FALSE(db.FindByUserNum(1, info));
    REQUIRE_FALSE(db.FindByUserName("alice", info));
}

TEST_CASE(SUITE("Delete nonexistent user returns false"))
{
    OutstationUserDatabase db;
    REQUIRE_FALSE(db.Delete("nobody"));
}

TEST_CASE(SUITE("AddOrUpdate overwrites existing user"))
{
    OutstationUserDatabase db;
    db.AddOrUpdate(MakeUser(1, "alice", UserRole::VIEWER));

    // update same user number with new info
    db.AddOrUpdate(MakeUser(1, "alice_updated", UserRole::OPERATOR));

    OutstationUserInfo info;
    REQUIRE(db.FindByUserNum(1, info));
    REQUIRE(info.userName == "alice_updated");
    REQUIRE(info.role == UserRole::OPERATOR);
}

TEST_CASE(SUITE("multiple users"))
{
    OutstationUserDatabase db;
    db.AddOrUpdate(MakeUser(1, "alice", UserRole::VIEWER));
    db.AddOrUpdate(MakeUser(2, "bob", UserRole::OPERATOR));
    db.AddOrUpdate(MakeUser(3, "charlie", UserRole::SINGLE_USER));

    OutstationUserInfo info;
    REQUIRE(db.FindByUserNum(1, info));
    REQUIRE(info.userName == "alice");
    REQUIRE(db.FindByUserNum(2, info));
    REQUIRE(info.userName == "bob");
    REQUIRE(db.FindByUserNum(3, info));
    REQUIRE(info.userName == "charlie");
}

TEST_CASE(SUITE("permissions are stored correctly"))
{
    OutstationUserDatabase db;
    auto perms = Permissions::Allowed(FunctionCode::READ, FunctionCode::WRITE);
    OutstationUserInfo user(10, "testuser", UserRole::UNDEFINED, 365, UpdateKey(0xBB, KeyWrapAlgorithm::AES_128),
                            perms);
    db.AddOrUpdate(user);

    OutstationUserInfo info;
    REQUIRE(db.FindByUserNum(10, info));
    REQUIRE(info.permissions.IsAllowed(FunctionCode::READ));
    REQUIRE(info.permissions.IsAllowed(FunctionCode::WRITE));
    REQUIRE_FALSE(info.permissions.IsAllowed(FunctionCode::OPERATE));
}
