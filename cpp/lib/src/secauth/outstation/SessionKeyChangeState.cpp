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
#include "secauth/outstation/SessionKeyChangeState.h"

#include "logging/LogMacros.h"

#include "opendnp3/logging/LogLevels.h"

#include <cstring>

namespace opendnp3
{

SessionKeyChangeState::SessionKeyChangeState(uint16_t challengeSize_, Logger logger_, ICryptoProvider& provider)
    : lastUserNum(0),
      challengeSize(AuthSizes::GetBoundedChallengeSize(challengeSize_)),
      logger(logger_),
      pProvider(&provider),
      keyChangeSeqNum(0)
{
    challengeData.fill(0);
}

bool SessionKeyChangeState::FormatKeyStatusResponse(HeaderWriter& writer,
                                                    uint16_t userNum,
                                                    HMACType hmacType,
                                                    KeyWrapAlgorithm keyWrapAlgo,
                                                    KeyStatus status,
                                                    const ser4cpp::rseq_t& hmac)
{
    this->lastUserNum = userNum;

    // Generate random challenge data
    ser4cpp::wseq_t challengeDest(challengeData.data(), challengeSize);
    std::error_code ec;
    auto random = pProvider->GetSecureRandom(challengeDest, ec);
    if (ec)
    {
        SIMPLE_LOG_BLOCK(logger, flags::ERR, ec.message().c_str());
        return false;
    }

    ++keyChangeSeqNum;

    statusRsp = Group120Var5(keyChangeSeqNum, userNum, keyWrapAlgo, status, hmacType, random, hmac);

    return writer.WriteFreeFormat(statusRsp);
}

bool SessionKeyChangeState::EqualsLastStatusResponse(const ser4cpp::rseq_t& object)
{
    // Reconstruct expected serialization and compare
    // For now, use simple field comparison
    Group120Var5 received;
    if (!received.Read(object))
    {
        return false;
    }

    return (received.keyChangeSeqNum == statusRsp.keyChangeSeqNum) && (received.userNum == statusRsp.userNum);
}

bool SessionKeyChangeState::CheckUserAndKSQMatches(uint16_t userNum, uint32_t keyChangeSeq)
{
    return (userNum == lastUserNum) && (keyChangeSeq == keyChangeSeqNum);
}

} // namespace opendnp3
