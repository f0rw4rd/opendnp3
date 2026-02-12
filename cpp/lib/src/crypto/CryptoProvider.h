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
#ifndef OPENDNP3_CRYPTOPROVIDER_H
#define OPENDNP3_CRYPTOPROVIDER_H

#include "crypto/AESKeyWrap.h"
#include "crypto/SHA1HMAC.h"
#include "crypto/SHA256HMAC.h"

#include "opendnp3/crypto/ICryptoProvider.h"
#include "opendnp3/util/Uncopyable.h"

namespace opendnp3
{

/**
 * OpenSSL-based implementation of ICryptoProvider.
 */
class CryptoProvider final : public ICryptoProvider, private Uncopyable
{
public:
    CryptoProvider() = default;

    virtual ser4cpp::rseq_t GetSecureRandom(ser4cpp::wseq_t& dest, std::error_code& ec) override;

    virtual IHMACAlgo& GetSHA1HMAC() override
    {
        return hmacSHA1;
    }

    virtual IHMACAlgo& GetSHA256HMAC() override
    {
        return hmacSHA256;
    }

    virtual IKeyWrapAlgo& GetAESKeyWrap() override
    {
        return keywrap;
    }

private:
    SHA1HMAC hmacSHA1;
    SHA256HMAC hmacSHA256;
    AESKeyWrap keywrap;
};

} // namespace opendnp3

#endif
