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
#ifndef OPENDNP3_MASTERSECURITY_H
#define OPENDNP3_MASTERSECURITY_H

#include "secauth/AuthSizes.h"
#include "secauth/SessionStore.h"
#include "secauth/master/MasterUserDatabase.h"

#include "opendnp3/crypto/ICryptoProvider.h"
#include "opendnp3/secauth/IMasterApplicationSA.h"
#include "opendnp3/secauth/MasterAuthSettings.h"
#include "opendnp3/util/Uncopyable.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>

namespace opendnp3
{

/**
 * Aggregates all security-related state for an SA5 master.
 */
class MasterSecurity : private Uncopyable
{
public:
    MasterSecurity(IMasterApplicationSA& application, const MasterAuthSettings& authSettings, ICryptoProvider& crypto);

    MasterAuthSettings settings;
    IMasterApplicationSA* pApplicationSA;
    ICryptoProvider* pCrypto;
    MasterUserDatabase userDB;
    SessionStore sessions;
    ser4cpp::rseq_t lastRequest;

    std::array<uint8_t, AuthSizes::MAX_MASTER_CHALLENGE_REPLY_FRAG_SIZE> challengeReplyBuffer;
};

} // namespace opendnp3

#endif
