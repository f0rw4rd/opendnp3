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
#ifndef OPENDNP3_USERSTATUSCHANGE_H
#define OPENDNP3_USERSTATUSCHANGE_H

#include "opendnp3/gen/KeyChangeMethod.h"
#include "opendnp3/gen/UserOperation.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <string>
#include <vector>

namespace opendnp3
{

struct Group120Var10; // forward declaration

/**
 * Public API type for initiating a User Status Change operation.
 * Maps to Group120Var10 on the wire.
 * Used by the master to add/delete/change users on an outstation.
 */
class UserStatusChange
{
public:
    UserStatusChange(KeyChangeMethod keyChangeMethod,
                     UserOperation userOperation,
                     uint32_t statusChangeSeqNum,
                     uint16_t userRole,
                     uint16_t userRoleExpDays,
                     const std::string& userName,
                     const ser4cpp::rseq_t& userPublicKey,
                     const ser4cpp::rseq_t& certificationData);

    /// Convert to wire format Group120Var10
    Group120Var10 Convert() const;

    KeyChangeMethod keyChangeMethod;
    UserOperation userOperation;
    uint32_t statusChangeSeqNum;
    uint16_t userRole;
    uint16_t userRoleExpDays;
    std::string userName;
    std::vector<uint8_t> userPublicKey;
    std::vector<uint8_t> certificationData;
};

} // namespace opendnp3

#endif
