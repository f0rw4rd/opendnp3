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
#ifndef OPENDNP3_CRYPTOERRORCODES_H
#define OPENDNP3_CRYPTOERRORCODES_H

#include <system_error>

namespace opendnp3
{
namespace crypto_errors
{

    enum Error : int
    {
        // HMAC errors
        HMAC_INSUFFICIENT_OUTPUT_BUFFER_SIZE,
        OPENSSL_HMAC_INIT_EX_ERROR,
        OPENSSL_HMAC_UPDATE_ERROR,
        OPENSSL_HMAC_FINAL_ERROR,

        // Secure random errors
        OPENSSL_RAND_BYTES_ERROR,

        // AES key wrap errors
        AES_SET_KEY_ERROR,
        AES_WRAPKEY_INPUT_NOT_DIV8,
        AES_WRAPKEY_INSUFFICIENT_OUTPUT_BUFFER,
        AES_WRAPKEY_ERROR,
        AES_UNWRAPKEY_INPUT_NOT_DIV8,
        AES_UNWRAPKEY_INSUFFICIENT_OUTPUT_BUFFER,
        AES_UNWRAPKEY_IV_ERROR,
        AES_UNWRAPKEY_PARAM_ERROR
    };

} // namespace crypto_errors

class CryptoErrorCategory final : public std::error_category
{
public:
    static const std::error_category& Instance()
    {
        return instance;
    }

    virtual const char* name() const noexcept override
    {
        return "opendnp3.crypto";
    }

    virtual std::string message(int ev) const override;

private:
    CryptoErrorCategory() {}
    CryptoErrorCategory(const CryptoErrorCategory&) = delete;

    static CryptoErrorCategory instance;
};

std::error_code make_error_code(crypto_errors::Error err);

} // namespace opendnp3

namespace std
{
template<> struct is_error_code_enum<opendnp3::crypto_errors::Error> : public true_type
{
};
} // namespace std

#endif
