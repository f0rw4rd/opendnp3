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
#include "crypto/GenericHMAC.h"

#include "crypto/CryptoErrorCodes.h"

#include <openssl/hmac.h>

namespace opendnp3
{

ser4cpp::rseq_t CalculateHMAC(const EVP_MD* md,
                              uint32_t outputSize,
                              const ser4cpp::rseq_t& key,
                              std::initializer_list<ser4cpp::rseq_t> data,
                              ser4cpp::wseq_t& output,
                              std::error_code& ec)
{
    if (output.length() < outputSize)
    {
        ec = make_error_code(crypto_errors::HMAC_INSUFFICIENT_OUTPUT_BUFFER_SIZE);
        return ser4cpp::rseq_t::empty();
    }

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // OpenSSL 3.x: HMAC_CTX is allocated on the heap
    HMAC_CTX* ctx = HMAC_CTX_new();
    if (!ctx)
    {
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_INIT_EX_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    if (HMAC_Init_ex(ctx, key, static_cast<int>(key.length()), md, nullptr) == 0)
    {
        HMAC_CTX_free(ctx);
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_INIT_EX_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    for (auto& bytes : data)
    {
        if (HMAC_Update(ctx, bytes, bytes.length()) == 0)
        {
            HMAC_CTX_free(ctx);
            ec = make_error_code(crypto_errors::OPENSSL_HMAC_UPDATE_ERROR);
            return ser4cpp::rseq_t::empty();
        }
    }

    unsigned int length = 0;
    if (HMAC_Final(ctx, output, &length) == 0)
    {
        HMAC_CTX_free(ctx);
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_FINAL_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    HMAC_CTX_free(ctx);
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
    // OpenSSL 1.1.x
    HMAC_CTX* ctx = HMAC_CTX_new();
    if (!ctx)
    {
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_INIT_EX_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    if (HMAC_Init_ex(ctx, key, static_cast<int>(key.length()), md, nullptr) == 0)
    {
        HMAC_CTX_free(ctx);
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_INIT_EX_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    for (auto& bytes : data)
    {
        if (HMAC_Update(ctx, bytes, bytes.length()) == 0)
        {
            HMAC_CTX_free(ctx);
            ec = make_error_code(crypto_errors::OPENSSL_HMAC_UPDATE_ERROR);
            return ser4cpp::rseq_t::empty();
        }
    }

    unsigned int length = 0;
    if (HMAC_Final(ctx, output, &length) == 0)
    {
        HMAC_CTX_free(ctx);
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_FINAL_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    HMAC_CTX_free(ctx);
#else
    // OpenSSL 1.0.x fallback
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    if (HMAC_Init_ex(&ctx, key, static_cast<int>(key.length()), md, nullptr) == 0)
    {
        HMAC_CTX_cleanup(&ctx);
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_INIT_EX_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    for (auto& bytes : data)
    {
        if (HMAC_Update(&ctx, bytes, bytes.length()) == 0)
        {
            HMAC_CTX_cleanup(&ctx);
            ec = make_error_code(crypto_errors::OPENSSL_HMAC_UPDATE_ERROR);
            return ser4cpp::rseq_t::empty();
        }
    }

    unsigned int length = 0;
    if (HMAC_Final(&ctx, output, &length) == 0)
    {
        HMAC_CTX_cleanup(&ctx);
        ec = make_error_code(crypto_errors::OPENSSL_HMAC_FINAL_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    HMAC_CTX_cleanup(&ctx);
#endif

    auto ret = output.readonly().take(outputSize);
    output.advance(outputSize);
    return ret;
}

} // namespace opendnp3
