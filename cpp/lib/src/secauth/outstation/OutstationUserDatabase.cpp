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
#include "secauth/outstation/OutstationUserDatabase.h"

namespace opendnp3
{

bool OutstationUserDatabase::FindByUserNum(uint16_t userNum, OutstationUserInfo& info) const
{
    auto it = users.find(userNum);
    if (it != users.end())
    {
        info = it->second;
        return true;
    }
    return false;
}

bool OutstationUserDatabase::FindByUserName(const std::string& userName, OutstationUserInfo& info) const
{
    for (auto& pair : users)
    {
        if (pair.second.userName == userName)
        {
            info = pair.second;
            return true;
        }
    }
    return false;
}

void OutstationUserDatabase::AddOrUpdate(const OutstationUserInfo& info)
{
    users[info.userNum] = info;
}

bool OutstationUserDatabase::Delete(const std::string& userName)
{
    for (auto it = users.begin(); it != users.end(); ++it)
    {
        if (it->second.userName == userName)
        {
            users.erase(it);
            return true;
        }
    }
    return false;
}

bool OutstationUserDatabase::GetUpdateKey(uint16_t userNum, UpdateKey& key) const
{
    auto it = users.find(userNum);
    if (it != users.end())
    {
        key = it->second.updateKey;
        return key.IsValid();
    }
    return false;
}

} // namespace opendnp3
