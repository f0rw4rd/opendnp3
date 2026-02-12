/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OPENDNP3_IOUTSTATIONUSERDATABASE_H
#define OPENDNP3_IOUTSTATIONUSERDATABASE_H

#include "secauth/outstation/OutstationUserInfo.h"

#include "opendnp3/secauth/UpdateKey.h"

#include <string>

namespace opendnp3
{

/**
 * Abstract interface for the outstation user database.
 */
class IOutstationUserDatabase
{
public:
    virtual ~IOutstationUserDatabase() {}

    /// Look up a user by user number. Returns true if found.
    virtual bool FindByUserNum(uint16_t userNum, OutstationUserInfo& info) const = 0;

    /// Look up a user by name. Returns true if found.
    virtual bool FindByUserName(const std::string& userName, OutstationUserInfo& info) const = 0;

    /// Add or update a user entry
    virtual void AddOrUpdate(const OutstationUserInfo& info) = 0;

    /// Delete a user by name. Returns true if the user existed.
    virtual bool Delete(const std::string& userName) = 0;

    /// Get the update key for a user by user number. Returns true if found.
    virtual bool GetUpdateKey(uint16_t userNum, UpdateKey& key) const = 0;
};

} // namespace opendnp3

#endif
