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
#include "crypto/AESKeyWrap.h"

#include "crypto/CryptoErrorCodes.h"

#include <openssl/aes.h>

namespace opendnp3
{

ser4cpp::rseq_t AESKeyWrap::WrapKey(const ser4cpp::rseq_t& kek,
                                    const ser4cpp::rseq_t& input,
                                    ser4cpp::wseq_t& output,
                                    std::error_code& ec) const
{
    // Input must be pre-padded into 8-byte blocks
    if ((input.length() % 8) != 0)
    {
        ec = make_error_code(crypto_errors::AES_WRAPKEY_INPUT_NOT_DIV8);
        return ser4cpp::rseq_t::empty();
    }

    const uint32_t outputSize = static_cast<uint32_t>(input.length()) + 8;

    if (output.length() < outputSize)
    {
        ec = make_error_code(crypto_errors::AES_WRAPKEY_INSUFFICIENT_OUTPUT_BUFFER);
        return ser4cpp::rseq_t::empty();
    }

    const int keySizeBits = static_cast<int>(kek.length() * 8);

    AES_KEY key;
    if (AES_set_encrypt_key(kek, keySizeBits, &key))
    {
        ec = make_error_code(crypto_errors::AES_SET_KEY_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    // If iv is null, the default IV is used (RFC 3394)
    const int len = AES_wrap_key(&key, nullptr, output, input, static_cast<unsigned int>(input.length()));
    if (len > 0)
    {
        auto ret = output.readonly().take(outputSize);
        output.advance(outputSize);
        return ret;
    }
    else
    {
        ec = make_error_code(crypto_errors::AES_WRAPKEY_ERROR);
        return ser4cpp::rseq_t::empty();
    }
}

ser4cpp::rseq_t AESKeyWrap::UnwrapKey(const ser4cpp::rseq_t& kek,
                                      const ser4cpp::rseq_t& input,
                                      ser4cpp::wseq_t& output,
                                      std::error_code& ec) const
{
    // Input must be at least 16 bytes and divisible by 8
    if ((input.length() < 16) || (input.length() % 8 != 0))
    {
        ec = make_error_code(crypto_errors::AES_UNWRAPKEY_INPUT_NOT_DIV8);
        return ser4cpp::rseq_t::empty();
    }

    const uint32_t outputSize = static_cast<uint32_t>(input.length()) - 8;

    if (output.length() < outputSize)
    {
        ec = make_error_code(crypto_errors::AES_UNWRAPKEY_INSUFFICIENT_OUTPUT_BUFFER);
        return ser4cpp::rseq_t::empty();
    }

    const int keySizeBits = static_cast<int>(kek.length() * 8);

    AES_KEY key;
    if (AES_set_decrypt_key(kek, keySizeBits, &key))
    {
        ec = make_error_code(crypto_errors::AES_SET_KEY_ERROR);
        return ser4cpp::rseq_t::empty();
    }

    const int result = AES_unwrap_key(&key, nullptr, output, input, static_cast<unsigned int>(input.length()));

    if (result > 0)
    {
        auto ret = output.readonly().take(outputSize);
        output.advance(outputSize);
        return ret;
    }
    else
    {
        auto code = (result == 0) ? crypto_errors::AES_UNWRAPKEY_IV_ERROR : crypto_errors::AES_UNWRAPKEY_PARAM_ERROR;
        ec = make_error_code(code);
        return ser4cpp::rseq_t::empty();
    }
}

} // namespace opendnp3
