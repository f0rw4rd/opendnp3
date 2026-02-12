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
#ifndef OPENDNP3_IOAUTHSTATE_H
#define OPENDNP3_IOAUTHSTATE_H

#include "app/APDUHeader.h"
#include "gen/objects/Group120.h"

#include "opendnp3/logging/Logger.h"

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

class OAuthContext; // forward declaration

/**
 * Abstract outstation authentication state interface.
 * Each concrete state handles the various auth events and returns the next state.
 */
class IOAuthState
{
public:
    virtual ~IOAuthState() = default;

    virtual IOAuthState* OnRegularRequest(OAuthContext& ocontext,
                                          const ser4cpp::rseq_t& fragment,
                                          const APDUHeader& header,
                                          const ser4cpp::rseq_t& objects)
        = 0;

    virtual IOAuthState* OnAggModeRequest(OAuthContext& ocontext,
                                          const APDUHeader& header,
                                          const ser4cpp::rseq_t& objects,
                                          const Group120Var3& aggModeRequest)
        = 0;

    virtual IOAuthState* OnAuthChallenge(OAuthContext& ocontext,
                                         const APDUHeader& header,
                                         const Group120Var1& challenge)
        = 0;

    virtual IOAuthState* OnAuthReply(OAuthContext& ocontext, const APDUHeader& header, const Group120Var2& reply) = 0;

    virtual IOAuthState* OnRequestKeyStatus(OAuthContext& ocontext,
                                            const APDUHeader& header,
                                            const Group120Var4& status)
        = 0;

    virtual IOAuthState* OnChangeSessionKeys(OAuthContext& ocontext,
                                             const ser4cpp::rseq_t& fragment,
                                             const APDUHeader& header,
                                             const Group120Var6& change)
        = 0;

    virtual IOAuthState* OnChallengeTimeout(OAuthContext& ocontext) = 0;

    virtual const char* GetName() = 0;

protected:
    IOAuthState* IgnoreRegularRequest(Logger& logger);
    IOAuthState* IgnoreAggModeRequest(Logger& logger);
    IOAuthState* IgnoreAuthChallenge(Logger& logger);
    IOAuthState* IgnoreAuthReply(Logger& logger);
    IOAuthState* IgnoreRequestKeyStatus(Logger& logger, uint16_t user);
    IOAuthState* IgnoreChangeSessionKeys(Logger& logger, uint16_t user);
    IOAuthState* IgnoreChallengeTimeout(Logger& logger);
};

} // namespace opendnp3

#endif
