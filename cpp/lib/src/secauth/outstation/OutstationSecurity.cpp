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
#include "secauth/outstation/OutstationSecurity.h"

namespace opendnp3
{

OutstationSecurity::OutstationSecurity(const OutstationParams& params,
                                       const OutstationAuthSettings& authSettings,
                                       Logger logger,
                                       IOutstationApplicationSA& application,
                                       ICryptoProvider& crypto)
    : state(SecurityState::IDLE),
      settings(authSettings),
      challenge(authSettings.challengeSize, params.maxRxFragSize),
      hmac(crypto, authSettings.hmacMode),
      pApplication(&application),
      pCrypto(&crypto),
      sessionKeyChangeState(authSettings.sessionKeyChangeChallengeSize, logger, crypto),
      updateKeyChangeState(authSettings.updateKeyChangeChallengeSize, logger, crypto),
      sessions(authSettings.sessionKeyTimeout, authSettings.maxAuthMsgCount)
{
}

} // namespace opendnp3
