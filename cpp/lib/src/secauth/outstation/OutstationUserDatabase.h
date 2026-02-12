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
#ifndef OPENDNP3_OUTSTATIONUSERDATABASE_H
#define OPENDNP3_OUTSTATIONUSERDATABASE_H

#include "secauth/outstation/IOutstationUserDatabase.h"

#include <map>

namespace opendnp3
{

/**
 * Concrete in-memory implementation of IOutstationUserDatabase.
 */
class OutstationUserDatabase final : public IOutstationUserDatabase
{
public:
    virtual bool FindByUserNum(uint16_t userNum, OutstationUserInfo& info) const override;
    virtual bool FindByUserName(const std::string& userName, OutstationUserInfo& info) const override;
    virtual void AddOrUpdate(const OutstationUserInfo& info) override;
    virtual bool Delete(const std::string& userName) override;
    virtual bool GetUpdateKey(uint16_t userNum, UpdateKey& key) const override;

private:
    // Map from user number to user info
    std::map<uint16_t, OutstationUserInfo> users;
};

} // namespace opendnp3

#endif
