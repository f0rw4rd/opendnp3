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
#ifndef OPENDNP3_OUTSTATIONUSERINFO_H
#define OPENDNP3_OUTSTATIONUSERINFO_H

#include "secauth/outstation/Permissions.h"

#include "opendnp3/gen/UserRole.h"
#include "opendnp3/secauth/UpdateKey.h"

#include <string>

namespace opendnp3
{

/**
 * Metadata stored for each user in the outstation user database.
 */
struct OutstationUserInfo
{
    OutstationUserInfo() : userNum(0), role(UserRole::UNDEFINED), roleExpDays(0) {}

    OutstationUserInfo(uint16_t userNum_,
                       const std::string& userName_,
                       UserRole role_,
                       uint16_t roleExpDays_,
                       const UpdateKey& updateKey_,
                       const Permissions& permissions_)
        : userNum(userNum_),
          userName(userName_),
          role(role_),
          roleExpDays(roleExpDays_),
          updateKey(updateKey_),
          permissions(permissions_)
    {
    }

    uint16_t userNum;
    std::string userName;
    UserRole role;
    uint16_t roleExpDays;
    UpdateKey updateKey;
    Permissions permissions;
};

} // namespace opendnp3

#endif
