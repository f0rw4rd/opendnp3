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
#ifndef OPENDNP3_IOUTSTATIONAPPLICATIONSA_H
#define OPENDNP3_IOUTSTATIONAPPLICATIONSA_H

#include "opendnp3/gen/KeyChangeMethod.h"
#include "opendnp3/gen/UserRole.h"
#include "opendnp3/outstation/IOutstationApplication.h"
#include "opendnp3/secauth/UpdateKey.h"

#include <string>

namespace opendnp3
{

/**
 * Extends IOutstationApplication with SA5-specific callbacks.
 */
class IOutstationApplicationSA : public IOutstationApplication
{
public:
    virtual ~IOutstationApplicationSA() {}

    /**
     * Called when a new Status Change Sequence Number has been established.
     * The application should persist this value.
     *
     * @param scsn The new status change sequence number
     */
    virtual void OnNewSCSN(uint32_t scsn) = 0;

    /**
     * Called when a user should be deleted.
     *
     * @param userName Name of the user to delete
     */
    virtual void OnDeleteUser(const std::string& userName) = 0;

    /**
     * Called when a user should be added or updated.
     *
     * @param userName Name of the user
     * @param userNum User number
     * @param role User role
     * @param roleExpDays Role expiration in days (0 = never expires)
     * @param key The user's update key
     */
    virtual void OnAddOrUpdateUser(
        const std::string& userName, uint16_t userNum, UserRole role, uint16_t roleExpDays, const UpdateKey& key)
        = 0;
};

} // namespace opendnp3

#endif
