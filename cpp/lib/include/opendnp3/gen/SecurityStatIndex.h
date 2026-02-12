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
#ifndef OPENDNP3_SECURITYSTATINDEX_H
#define OPENDNP3_SECURITYSTATINDEX_H

#include <cstdint>

namespace opendnp3
{

/**
 * Indices of the SA security statistics (IEEE 1815-2012 Table 8-7)
 */
enum class SecurityStatIndex : uint8_t
{
    UNEXPECTED_MESSAGES = 0,
    AUTHORIZATION_FAILURES = 1,
    AUTHENTICATION_FAILURES = 2,
    REPLY_TIMEOUTS = 3,
    REKEYS_DUE_TO_AUTH_FAILURE = 4,
    TOTAL_MESSAGES_TX = 5,
    TOTAL_MESSAGES_RX = 6,
    CRITICAL_MESSAGES_TX = 7,
    CRITICAL_MESSAGES_RX = 8,
    DISCARDED_MESSAGES = 9,
    ERROR_MESSAGES_TX = 10,
    ERROR_MESSAGES_RX = 11,
    SUCCESSFUL_AUTHS = 12,
    SESSION_KEY_CHANGES = 13,
    FAILED_SESSION_KEY_CHANGES = 14,
    UPDATE_KEY_CHANGES = 15,
    FAILED_UPDATE_KEY_CHANGES = 16,
    REKEYS_DUE_TO_RESTART = 17
};

/// Total number of defined security statistics
static const uint8_t NUM_SECURITY_STATS = 18;

} // namespace opendnp3

#endif
