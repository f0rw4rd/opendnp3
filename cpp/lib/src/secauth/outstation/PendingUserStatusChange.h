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
#ifndef OPENDNP3_PENDINGUSERSTATUSCHANGE_H
#define OPENDNP3_PENDINGUSERSTATUSCHANGE_H

#include "opendnp3/gen/KeyChangeMethod.h"
#include "opendnp3/gen/UserOperation.h"
#include "opendnp3/gen/UserRole.h"
#include "opendnp3/util/Uncopyable.h"

#include <cstdint>
#include <map>
#include <string>

namespace opendnp3
{

class ChangeData
{
public:
    ChangeData() : keyChangeMethod(KeyChangeMethod::UNDEFINED), userRole(UserRole::UNDEFINED), expirationDays(0) {}

    ChangeData(KeyChangeMethod keyChangeMethod_, UserRole userRole_, uint16_t expirationDays_)
        : keyChangeMethod(keyChangeMethod_), userRole(userRole_), expirationDays(expirationDays_)
    {
    }

    KeyChangeMethod keyChangeMethod;
    UserRole userRole;
    uint16_t expirationDays;
};

class PendingUserStatusChanges : private Uncopyable
{
public:
    bool IsPending(const std::string& userName) const;

    void QueueChange(const std::string& userName, const ChangeData& data);

    bool PopChange(const std::string& userName, ChangeData& data);

private:
    typedef std::map<std::string, ChangeData> ChangeMap;
    ChangeMap changeMap;
};

} // namespace opendnp3

#endif
