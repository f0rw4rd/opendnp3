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
#ifndef OPENDNP3_MASTERUSERDATABASE_H
#define OPENDNP3_MASTERUSERDATABASE_H

#include "secauth/master/IMasterUserDatabase.h"

#include <map>

namespace opendnp3
{

/**
 * Concrete in-memory implementation of IMasterUserDatabase.
 */
class MasterUserDatabase final : public IMasterUserDatabase
{
public:
    virtual bool GetUpdateKey(const std::string& userName, UpdateKey& key) const override;
    virtual void AddOrUpdate(const std::string& userName, uint16_t userNum, const UpdateKey& key) override;
    virtual bool GetUserNum(const std::string& userName, uint16_t& userNum) const override;

private:
    struct UserEntry
    {
        uint16_t userNum;
        UpdateKey key;
    };
    std::map<std::string, UserEntry> users;
};

} // namespace opendnp3

#endif
