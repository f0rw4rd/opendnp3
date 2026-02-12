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
#ifndef OPENDNP3_IKEYWRAPALGO_H
#define OPENDNP3_IKEYWRAPALGO_H

#include <ser4cpp/container/SequenceTypes.h>

#include <system_error>

namespace opendnp3
{

/**
 * Abstract interface for AES Key Wrap (RFC 3394).
 * Must work with any valid AES key size: 128, 192, or 256 bits.
 * Input must be pre-padded into 8-byte blocks.
 */
class IKeyWrapAlgo
{
public:
    virtual ~IKeyWrapAlgo() {}

    /**
     * Wrap a key using AES Key Wrap.
     * @param kek Key Encryption Key
     * @param input Plaintext key data (must be multiple of 8 bytes)
     * @param dest Output buffer (must be >= input.length() + 8)
     * @param ec Set on failure
     * @return Read-only view of the wrapped key data
     */
    virtual ser4cpp::rseq_t WrapKey(const ser4cpp::rseq_t& kek,
                                    const ser4cpp::rseq_t& input,
                                    ser4cpp::wseq_t& dest,
                                    std::error_code& ec) const
        = 0;

    /**
     * Unwrap a key using AES Key Wrap.
     * @param kek Key Encryption Key
     * @param input Wrapped key data (must be multiple of 8 bytes, >= 16 bytes)
     * @param dest Output buffer (must be >= input.length() - 8)
     * @param ec Set on failure
     * @return Read-only view of the unwrapped key data
     */
    virtual ser4cpp::rseq_t UnwrapKey(const ser4cpp::rseq_t& kek,
                                      const ser4cpp::rseq_t& input,
                                      ser4cpp::wseq_t& dest,
                                      std::error_code& ec) const
        = 0;
};

} // namespace opendnp3

#endif
