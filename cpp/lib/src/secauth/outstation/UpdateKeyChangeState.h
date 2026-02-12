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
#ifndef OPENDNP3_UPDATEKEYCHANGESTATE_H
#define OPENDNP3_UPDATEKEYCHANGESTATE_H

#include "app/HeaderWriter.h"
#include "secauth/AuthSizes.h"
#include "secauth/outstation/IOutstationUserDatabase.h"

#include "opendnp3/crypto/ICryptoProvider.h"
#include "opendnp3/logging/Logger.h"
#include "opendnp3/secauth/UpdateKey.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace opendnp3
{

/**
 * Tracks the state of an update key change procedure on the outstation side.
 */
class UpdateKeyChangeState
{
public:
    struct VerificationData
    {
        VerificationData() : keyChangeSeqNum(0), userNum(0) {}

        VerificationData(std::string username_,
                         ser4cpp::rseq_t masterChallenge_,
                         ser4cpp::rseq_t outstationChallenge_,
                         uint32_t keyChangeSeqNum_,
                         uint16_t userNum_)
            : username(std::move(username_)),
              masterChallenge(masterChallenge_),
              outstationChallenge(outstationChallenge_),
              keyChangeSeqNum(keyChangeSeqNum_),
              userNum(userNum_)
        {
        }

        std::string username;
        ser4cpp::rseq_t masterChallenge;
        ser4cpp::rseq_t outstationChallenge;
        uint32_t keyChangeSeqNum;
        uint16_t userNum;
    };

    UpdateKeyChangeState(uint16_t challengeSize, Logger logger, ICryptoProvider& provider);

    void Reset();

    /// Write the update key change response (Group120Var12) to the writer
    bool WriteUpdateKeyChangeResponse(HeaderWriter& writer,
                                      uint32_t ksq,
                                      const std::string& username,
                                      const ser4cpp::rseq_t& masterChallengeData,
                                      uint16_t freeUserNum);

    /// Verify that the user and KSQ match and return the verification data
    bool VerifyUserAndKSQ(uint32_t ksq, uint16_t userNum, VerificationData& data);

private:
    bool m_valid;
    VerificationData m_data;

    const uint16_t M_CHALLENGE_SIZE;
    Logger m_logger;
    ICryptoProvider* m_crypto;

    std::array<uint8_t, AuthSizes::MAX_CHALLENGE_DATA_SIZE> m_masterChallengeBuffer;
    std::array<uint8_t, AuthSizes::MAX_CHALLENGE_DATA_SIZE> m_outstationChallengeBuffer;
};

} // namespace opendnp3

#endif
