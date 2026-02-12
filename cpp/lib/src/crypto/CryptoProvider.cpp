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
#include "crypto/CryptoProvider.h"

#include "crypto/CryptoErrorCodes.h"

#include <openssl/rand.h>

namespace opendnp3
{

ser4cpp::rseq_t CryptoProvider::GetSecureRandom(ser4cpp::wseq_t& dest, std::error_code& ec)
{
    if (RAND_bytes(dest, static_cast<int>(dest.length())) != 1)
    {
        ec = make_error_code(crypto_errors::OPENSSL_RAND_BYTES_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    auto ret = dest.readonly();
    dest.advance(dest.length());
    return ret;
}

} // namespace opendnp3
