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
#ifndef OPENDNP3_OUTSTATIONSECURITY_H
#define OPENDNP3_OUTSTATIONSECURITY_H

#include "secauth/DeferredASDU.h"
#include "secauth/HMACProvider.h"
#include "secauth/SessionStore.h"
#include "secauth/outstation/AuthorityCredentials.h"
#include "secauth/outstation/ChallengeState.h"
#include "secauth/outstation/OutstationUserDatabase.h"
#include "secauth/outstation/PendingUserStatusChange.h"
#include "secauth/outstation/SessionKeyChangeState.h"
#include "secauth/outstation/Statistics.h"
#include "secauth/outstation/UnofficialStatistics.h"
#include "secauth/outstation/UpdateKeyChangeState.h"

#include "opendnp3/crypto/ICryptoProvider.h"
#include "opendnp3/logging/Logger.h"
#include "opendnp3/outstation/OutstationParams.h"
#include "opendnp3/secauth/IOutstationApplicationSA.h"
#include "opendnp3/secauth/OutstationAuthSettings.h"

namespace opendnp3
{

enum class SecurityState
{
    IDLE,
    WAIT_FOR_REPLY
};

/**
 * Aggregates all security-related state for an SA5 outstation.
 */
class OutstationSecurity
{
public:
    OutstationSecurity(const OutstationParams& params,
                       const OutstationAuthSettings& settings,
                       Logger logger,
                       IOutstationApplicationSA& application,
                       ICryptoProvider& crypto);

    SecurityState state;
    OutstationAuthSettings settings;
    ChallengeState challenge;
    HMACProvider hmac;
    DeferredASDU deferred;
    IOutstationApplicationSA* pApplication;
    OutstationUserDatabase userDB;
    ICryptoProvider* pCrypto;
    SessionKeyChangeState sessionKeyChangeState;
    UpdateKeyChangeState updateKeyChangeState;
    SessionStore sessions;
    Statistics stats;
    UnofficialStatistics otherStats;
    AuthorityCredentials credentials;
    PendingUserStatusChanges statusChanges;
};

} // namespace opendnp3

#endif
