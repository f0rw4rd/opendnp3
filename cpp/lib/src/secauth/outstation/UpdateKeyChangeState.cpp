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
#include "secauth/outstation/UpdateKeyChangeState.h"

#include "gen/objects/Group120.h"
#include "logging/LogMacros.h"

#include "opendnp3/logging/LogLevels.h"

#include <cstring>

namespace opendnp3
{

UpdateKeyChangeState::UpdateKeyChangeState(uint16_t challengeSize, Logger logger, ICryptoProvider& provider)
    : m_valid(false),
      M_CHALLENGE_SIZE(AuthSizes::GetBoundedChallengeSize(challengeSize)),
      m_logger(logger),
      m_crypto(&provider)
{
    m_masterChallengeBuffer.fill(0);
    m_outstationChallengeBuffer.fill(0);
}

void UpdateKeyChangeState::Reset()
{
    m_valid = false;
}

bool UpdateKeyChangeState::WriteUpdateKeyChangeResponse(HeaderWriter& writer,
                                                        uint32_t ksq,
                                                        const std::string& username,
                                                        const ser4cpp::rseq_t& masterChallengeData,
                                                        uint16_t freeUserNum)
{
    // Copy master challenge data to our buffer
    auto copyLen = std::min(static_cast<size_t>(masterChallengeData.length()), m_masterChallengeBuffer.size());
    std::memcpy(m_masterChallengeBuffer.data(), masterChallengeData, copyLen);
    auto masterChallenge = ser4cpp::rseq_t(m_masterChallengeBuffer.data(), static_cast<uint32_t>(copyLen));

    // Generate outstation challenge data
    ser4cpp::wseq_t outstationDest(m_outstationChallengeBuffer.data(), M_CHALLENGE_SIZE);
    std::error_code ec;
    auto outstationChallenge = m_crypto->GetSecureRandom(outstationDest, ec);
    if (ec)
    {
        SIMPLE_LOG_BLOCK(m_logger, flags::ERR, ec.message().c_str());
        return false;
    }

    Group120Var12 reply(ksq, freeUserNum, outstationChallenge);
    if (!writer.WriteFreeFormat(reply))
    {
        SIMPLE_LOG_BLOCK(m_logger, flags::ERR, "Insufficient space for update key change response");
        return false;
    }

    // Store verification data
    m_data = VerificationData(username, masterChallenge, outstationChallenge, ksq, freeUserNum);
    m_valid = true;

    return true;
}

bool UpdateKeyChangeState::VerifyUserAndKSQ(uint32_t ksq, uint16_t userNum, VerificationData& data)
{
    if (!m_valid)
    {
        return false;
    }

    if ((ksq != m_data.keyChangeSeqNum) || (userNum != m_data.userNum))
    {
        return false;
    }

    data = m_data;
    return true;
}

} // namespace opendnp3
