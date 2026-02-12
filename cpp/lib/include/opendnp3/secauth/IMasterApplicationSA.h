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
#ifndef OPENDNP3_IMASTERAPPLICATIONSA_H
#define OPENDNP3_IMASTERAPPLICATIONSA_H

#include "opendnp3/master/IMasterApplication.h"
#include "opendnp3/secauth/UpdateKey.h"

#include <string>

namespace opendnp3
{

/**
 * Extends IMasterApplication with SA5-specific callbacks.
 */
class IMasterApplicationSA : public IMasterApplication
{
public:
    virtual ~IMasterApplicationSA() {}

    /**
     * Called when a new update key has been established for a user.
     * The application should persist this key securely.
     *
     * @param userName Name of the user
     * @param userNum User number
     * @param key The new update key
     */
    virtual void PersistNewUpdateKey(const std::string& userName, uint16_t userNum, const UpdateKey& key) = 0;
};

} // namespace opendnp3

#endif
