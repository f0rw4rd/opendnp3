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
#include "secauth/master/MasterUserDatabase.h"

namespace opendnp3
{

bool MasterUserDatabase::GetUpdateKey(const std::string& userName, UpdateKey& key) const
{
    auto it = users.find(userName);
    if (it != users.end())
    {
        key = it->second.key;
        return key.IsValid();
    }
    return false;
}

void MasterUserDatabase::AddOrUpdate(const std::string& userName, uint16_t userNum, const UpdateKey& key)
{
    users[userName] = {userNum, key};
}

bool MasterUserDatabase::GetUserNum(const std::string& userName, uint16_t& userNum) const
{
    auto it = users.find(userName);
    if (it != users.end())
    {
        userNum = it->second.userNum;
        return true;
    }
    return false;
}

} // namespace opendnp3
