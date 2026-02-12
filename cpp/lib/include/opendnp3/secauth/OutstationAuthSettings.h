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
#ifndef OPENDNP3_OUTSTATIONAUTHSETTINGS_H
#define OPENDNP3_OUTSTATIONAUTHSETTINGS_H

#include "opendnp3/secauth/AuthConstants.h"
#include "opendnp3/secauth/CriticalFunctions.h"
#include "opendnp3/secauth/HMACMode.h"
#include "opendnp3/secauth/StatThresholds.h"
#include "opendnp3/util/TimeDuration.h"

#include <string>

namespace opendnp3
{

/**
 * Configuration for SA5 outstation authentication.
 */
struct OutstationAuthSettings
{
    OutstationAuthSettings()
        : outstationName("outstation"),
          challengeTimeout(TimeDuration::Seconds(2)),
          challengeSize(4),
          sessionKeyChangeChallengeSize(4),
          updateKeyChangeChallengeSize(4),
          assocId(0),
          hmacMode(HMACMode::SHA256_TRUNC_8),
          functions(CriticalFunctions::AuthOptional()),
          maxAuthMsgCount(AuthConstants::DEFAULT_SESSION_KEY_MAX_AUTH_COUNT),
          sessionKeyTimeout(TimeDuration::Minutes(AuthConstants::DEFAULT_SESSION_KEY_CHANGE_MINUTES))
    {
    }

    /// Organizationally unique outstation name (important security parameter)
    std::string outstationName;
    /// Timeout for challenge responses
    TimeDuration challengeTimeout;
    /// Number of bytes in a standard session challenge
    uint16_t challengeSize;
    /// Number of bytes in a session key change challenge
    uint16_t sessionKeyChangeChallengeSize;
    /// Number of bytes in an update key change challenge
    uint16_t updateKeyChangeChallengeSize;
    /// Association ID reported in transmissions
    uint16_t assocId;
    /// HMAC mode for challenges
    HMACMode hmacMode;
    /// Defines which function codes are deemed critical
    CriticalFunctions functions;
    /// Max authenticated messages before session key change required
    uint32_t maxAuthMsgCount;
    /// Maximum time before session key expires
    TimeDuration sessionKeyTimeout;
    /// Deadband thresholds for security statistics
    StatThresholds statThresholds;
};

} // namespace opendnp3

#endif
