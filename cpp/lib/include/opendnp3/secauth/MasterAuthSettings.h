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
#ifndef OPENDNP3_MASTERAUTHSETTINGS_H
#define OPENDNP3_MASTERAUTHSETTINGS_H

#include "opendnp3/secauth/AuthConstants.h"
#include "opendnp3/secauth/HMACMode.h"
#include "opendnp3/util/TimeDuration.h"

namespace opendnp3
{

/**
 * Configuration for SA5 master authentication.
 */
struct MasterAuthSettings
{
    MasterAuthSettings()
        : challengeTimeout(TimeDuration::Seconds(2)),
          challengeSize(4),
          hmacMode(HMACMode::SHA256_TRUNC_8),
          maxAuthMsgCount(AuthConstants::DEFAULT_SESSION_KEY_MAX_AUTH_COUNT),
          sessionKeyTimeout(TimeDuration::Minutes(AuthConstants::DEFAULT_SESSION_KEY_CHANGE_MINUTES)),
          sessionChangeInterval(TimeDuration::Minutes(10))
    {
    }

    /// Timeout for challenge responses
    TimeDuration challengeTimeout;
    /// Number of bytes in a challenge
    uint16_t challengeSize;
    /// HMAC mode to use when challenging
    HMACMode hmacMode;
    /// Max authenticated messages before session key change required
    uint32_t maxAuthMsgCount;
    /// Maximum time before session key expires
    TimeDuration sessionKeyTimeout;
    /// Interval at which session keys are refreshed
    TimeDuration sessionChangeInterval;
};

} // namespace opendnp3

#endif
