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
#ifndef OPENDNP3_FINISHUPDATEKEYCHANGEARGS_H
#define OPENDNP3_FINISHUPDATEKEYCHANGEARGS_H

#include "opendnp3/secauth/UpdateKey.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>
#include <string>
#include <vector>

namespace opendnp3
{

/**
 * Arguments required for completing an update key change on the master.
 * Aggregates all the data from the BeginUpdateKeyChange step plus
 * the encrypted key data provided by the authority.
 */
class FinishUpdateKeyChangeArgs
{
public:
    FinishUpdateKeyChangeArgs(const std::string& username,
                              const std::string& outstationName,
                              uint16_t userNum,
                              uint32_t keyChangeSequenceNumber,
                              const ser4cpp::rseq_t& masterChallengeData,
                              const ser4cpp::rseq_t& outstationChallengeData,
                              const ser4cpp::rseq_t& encryptedKeyData,
                              const UpdateKey& key);

    /// The UTF-8 username shared by the authority and outstation
    std::string username;

    /// Organizationally unique name of the outstation
    std::string outstationName;

    /// The user number assigned by the outstation
    uint16_t userNum;

    /// The KSQ specified by the outstation
    uint32_t keyChangeSequenceNum;

    /// The challenge data that the master chose when it initiated the request
    std::vector<uint8_t> masterChallengeData;

    /// The challenge data that the outstation provided in its response
    std::vector<uint8_t> outstationChallengeData;

    /// The encrypted key data provided by the authority
    std::vector<uint8_t> encryptedKeyData;

    /// The plaintext update key derived by the master and signed by the authority
    UpdateKey updateKey;
};

} // namespace opendnp3

#endif
