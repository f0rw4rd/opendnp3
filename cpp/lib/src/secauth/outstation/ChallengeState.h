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
#ifndef OPENDNP3_CHALLENGESTATE_H
#define OPENDNP3_CHALLENGESTATE_H

#include "app/APDUHeader.h"
#include "app/APDUResponse.h"
#include "secauth/AuthSizes.h"
#include "secauth/DeferredASDU.h"
#include "secauth/HMACProvider.h"

#include "opendnp3/crypto/ICryptoProvider.h"
#include "opendnp3/gen/HMACType.h"
#include "opendnp3/logging/Logger.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <array>
#include <cstdint>

namespace opendnp3
{

/**
 * Tracks challenge/response state for the outstation side of SA5.
 * Stores the critical ASDU, generates challenge data, and verifies HMACs.
 */
class ChallengeState
{
public:
    ChallengeState(uint16_t challengeSize, uint32_t maxRxASDUSize);

    /// Generate and write a challenge to the response, storing the critical ASDU
    bool WriteChallenge(const ser4cpp::rseq_t& fragment,
                        const APDUHeader& header,
                        APDUResponse& response,
                        HMACType hmacType,
                        ICryptoProvider& crypto,
                        Logger* pLogger);

    /// Verify an HMAC against the stored challenge and critical ASDU
    bool VerifyAuthenticity(const ser4cpp::rseq_t& key,
                            HMACProvider& provider,
                            const ser4cpp::rseq_t& hmac,
                            Logger logger);

    /// Get the stored critical ASDU fragment
    ser4cpp::rseq_t GetCriticalASDU() const;

    /// Get the stored critical ASDU header
    APDUHeader GetCriticalHeader() const;

private:
    const uint16_t CHALLENGE_SIZE;

    DeferredASDU criticalASDU;

    ser4cpp::rseq_t challengeFragment;
    std::array<uint8_t, AuthSizes::MAX_OUTSTATION_CHALLENGE_RESPONSE_FRAGMENT_SIZE> challengeFragmentBuffer;

    ser4cpp::rseq_t challengeData;
    std::array<uint8_t, AuthSizes::MAX_CHALLENGE_DATA_SIZE> challengeDataBuffer;

    uint32_t seqNumber;
};

} // namespace opendnp3

#endif
