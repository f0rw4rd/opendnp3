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
#ifndef OPENDNP3_SECAUTH_CRYPTO_H
#define OPENDNP3_SECAUTH_CRYPTO_H

#include "opendnp3/gen/HMACType.h"
#include "opendnp3/secauth/HMACMode.h"

namespace opendnp3
{

/// Static helpers on top of ICryptoProvider
class Crypto
{
public:
    Crypto() = delete;

    /// Try to convert an HMACType enum from the wire to our internal HMACMode
    static bool TryGetHMACMode(HMACType type, HMACMode& mode);
};

} // namespace opendnp3

#endif
