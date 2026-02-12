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
#include "secauth/outstation/IOAuthState.h"

#include "logging/LogMacros.h"

#include "opendnp3/logging/LogLevels.h"

namespace opendnp3
{

IOAuthState* IOAuthState::IgnoreRegularRequest(Logger& logger)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring regular request", this->GetName());
    return this;
}

IOAuthState* IOAuthState::IgnoreAggModeRequest(Logger& logger)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring Aggressive mode request", this->GetName());
    return this;
}

IOAuthState* IOAuthState::IgnoreAuthChallenge(Logger& logger)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring Auth challenge", this->GetName());
    return this;
}

IOAuthState* IOAuthState::IgnoreAuthReply(Logger& logger)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring Auth reply", this->GetName());
    return this;
}

IOAuthState* IOAuthState::IgnoreRequestKeyStatus(Logger& logger, uint16_t user)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring key status request for user %u", this->GetName(),
                     user);
    return this;
}

IOAuthState* IOAuthState::IgnoreChangeSessionKeys(Logger& logger, uint16_t user)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring change session keys for user %u", this->GetName(),
                     user);
    return this;
}

IOAuthState* IOAuthState::IgnoreChallengeTimeout(Logger& logger)
{
    FORMAT_LOG_BLOCK(logger, flags::WARN, "AuthState: %s - Ignoring challenge timeout", this->GetName());
    return this;
}

} // namespace opendnp3
