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
#ifndef OPENDNP3_IMASTERUSERDATABASE_H
#define OPENDNP3_IMASTERUSERDATABASE_H

#include "opendnp3/secauth/UpdateKey.h"

#include <string>

namespace opendnp3
{

/**
 * Abstract interface for the master-side user database.
 * Maps user names/numbers to update keys.
 */
class IMasterUserDatabase
{
public:
    virtual ~IMasterUserDatabase() {}

    /// Get the update key for a user. Returns true if found.
    virtual bool GetUpdateKey(const std::string& userName, UpdateKey& key) const = 0;

    /// Add or update a user's update key
    virtual void AddOrUpdate(const std::string& userName, uint16_t userNum, const UpdateKey& key) = 0;

    /// Get the user number for a user name. Returns true if found.
    virtual bool GetUserNum(const std::string& userName, uint16_t& userNum) const = 0;
};

} // namespace opendnp3

#endif
