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
#include "crypto/CryptoErrorCodes.h"

namespace opendnp3
{

CryptoErrorCategory CryptoErrorCategory::instance;

std::string CryptoErrorCategory::message(int ev) const
{
    switch (ev)
    {
    case (crypto_errors::HMAC_INSUFFICIENT_OUTPUT_BUFFER_SIZE):
        return "insufficient output buffer size for HMAC calculation";
    case (crypto_errors::OPENSSL_HMAC_INIT_EX_ERROR):
        return "openssl: failure calling HMAC_Init_ex()";
    case (crypto_errors::OPENSSL_HMAC_UPDATE_ERROR):
        return "openssl: failure calling HMAC_Update()";
    case (crypto_errors::OPENSSL_HMAC_FINAL_ERROR):
        return "openssl: failure calling HMAC_Final()";
    case (crypto_errors::OPENSSL_RAND_BYTES_ERROR):
        return "openssl: failure calling RAND_bytes()";
    case (crypto_errors::AES_SET_KEY_ERROR):
        return "openssl: error setting the AES key";
    case (crypto_errors::AES_WRAPKEY_INPUT_NOT_DIV8):
        return "input to WrapKey() not divisible by 8";
    case (crypto_errors::AES_WRAPKEY_INSUFFICIENT_OUTPUT_BUFFER):
        return "insufficient output buffer size for WrapKey()";
    case (crypto_errors::AES_WRAPKEY_ERROR):
        return "openssl: failure calling AES_wrap_key()";
    case (crypto_errors::AES_UNWRAPKEY_INPUT_NOT_DIV8):
        return "input to UnwrapKey() not divisible by 8";
    case (crypto_errors::AES_UNWRAPKEY_INSUFFICIENT_OUTPUT_BUFFER):
        return "insufficient output buffer size for UnwrapKey()";
    case (crypto_errors::AES_UNWRAPKEY_IV_ERROR):
        return "openssl: decryption of key data failed in AES_unwrap_key()";
    case (crypto_errors::AES_UNWRAPKEY_PARAM_ERROR):
        return "openssl: failure calling AES_unwrap_key()";
    default:
        return "unknown crypto error";
    }
}

std::error_code make_error_code(crypto_errors::Error err)
{
    return std::error_code(err, CryptoErrorCategory::Instance());
}

} // namespace opendnp3
