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
#ifndef OPENDNP3_KEYCHANGECONFIRMATIONHMAC_H
#define OPENDNP3_KEYCHANGECONFIRMATIONHMAC_H

#include "secauth/AuthSizes.h"

#include "opendnp3/crypto/IHMACAlgo.h"

#include <array>
#include <cstdint>
#include <system_error>

namespace opendnp3
{

/**
 * Computes the HMAC used to confirm a session key change or update key change.
 * The HMAC is computed over: KSQ || user number || challenge data from both sides
 */
class KeyChangeConfirmationHMAC
{
public:
    /**
     * Compute the key change confirmation HMAC.
     *
     * @param algo HMAC algorithm to use
     * @param key The session or update key to use as HMAC key
     * @param keyChangeSeqNum Key change sequence number
     * @param userNum User number
     * @param outstationChallengeData Challenge data from outstation
     * @param masterChallengeData Challenge data from master
     * @param ec Set on error
     * @return Read-only view of the computed HMAC
     */
    ser4cpp::rseq_t Compute(IHMACAlgo& algo,
                            const ser4cpp::rseq_t& key,
                            uint32_t keyChangeSeqNum,
                            uint16_t userNum,
                            const ser4cpp::rseq_t& outstationChallengeData,
                            const ser4cpp::rseq_t& masterChallengeData,
                            std::error_code& ec);

private:
    std::array<uint8_t, AuthSizes::MAX_HMAC_OUTPUT_SIZE> m_buffer;
};

} // namespace opendnp3

#endif
