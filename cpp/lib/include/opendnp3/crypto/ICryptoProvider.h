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
#ifndef OPENDNP3_ICRYPTOPROVIDER_H
#define OPENDNP3_ICRYPTOPROVIDER_H

#include "opendnp3/crypto/IHMACAlgo.h"
#include "opendnp3/crypto/IKeyWrapAlgo.h"
#include "opendnp3/crypto/ISecureRandom.h"

namespace opendnp3
{

/**
 * Main provider interface for cryptographic services needed by SA5.
 * Inherits from ISecureRandom for random number generation.
 * All methods are assumed to be thread-safe.
 *
 * The default implementation is based on OpenSSL.
 */
class ICryptoProvider : public ISecureRandom
{
public:
    virtual ~ICryptoProvider() {}

    /// Get the HMAC-SHA-1 algorithm implementation
    virtual IHMACAlgo& GetSHA1HMAC() = 0;

    /// Get the HMAC-SHA-256 algorithm implementation
    virtual IHMACAlgo& GetSHA256HMAC() = 0;

    /// Get the AES Key Wrap algorithm implementation
    virtual IKeyWrapAlgo& GetAESKeyWrap() = 0;
};

} // namespace opendnp3

#endif
