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
#include "opendnp3/secauth/StatThresholds.h"

#include <cstring>
#include <limits>

namespace opendnp3
{

const uint32_t StatThresholds::DEFAULTS[NUM_SECURITY_STATS] = {
    1, // UNEXPECTED_MESSAGES
    1, // AUTHORIZATION_FAILURES
    1, // AUTHENTICATION_FAILURES
    1, // REPLY_TIMEOUTS
    1, // REKEYS_DUE_TO_AUTH_FAILURE
    1, // TOTAL_MESSAGES_TX
    1, // TOTAL_MESSAGES_RX
    1, // CRITICAL_MESSAGES_TX
    1, // CRITICAL_MESSAGES_RX
    1, // DISCARDED_MESSAGES
    1, // ERROR_MESSAGES_TX
    1, // ERROR_MESSAGES_RX
    1, // SUCCESSFUL_AUTHS
    1, // SESSION_KEY_CHANGES
    1, // FAILED_SESSION_KEY_CHANGES
    1, // UPDATE_KEY_CHANGES
    1, // FAILED_UPDATE_KEY_CHANGES
    1  // REKEYS_DUE_TO_RESTART
};

StatThresholds::StatThresholds()
{
    std::memcpy(thresholds, DEFAULTS, sizeof(thresholds));
}

uint32_t StatThresholds::GetDeadband(uint16_t index) const
{
    if (index >= NUM_SECURITY_STATS)
    {
        return std::numeric_limits<uint32_t>::max();
    }
    return thresholds[index];
}

void StatThresholds::Set(SecurityStatIndex index, uint32_t threshold)
{
    auto idx = static_cast<uint8_t>(index);
    if (idx < NUM_SECURITY_STATS)
    {
        thresholds[idx] = threshold;
    }
}

} // namespace opendnp3
