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
#ifndef OPENDNP3_HMACPROVIDER_H
#define OPENDNP3_HMACPROVIDER_H

#include "secauth/AuthSizes.h"

#include "opendnp3/crypto/ICryptoProvider.h"
#include "opendnp3/gen/HMACType.h"
#include "opendnp3/secauth/HMACMode.h"

#include <array>
#include <cstdint>
#include <initializer_list>
#include <system_error>

namespace opendnp3
{

/**
 * Wraps an ICryptoProvider to compute HMACs with the configured truncation mode.
 */
class HMACProvider
{
public:
    HMACProvider(ICryptoProvider& provider, HMACMode mode);

    HMACType GetType() const;

    /// Compute HMAC and return a truncated view of the result
    ser4cpp::rseq_t Compute(const ser4cpp::rseq_t& key,
                            std::initializer_list<ser4cpp::rseq_t> buffers,
                            std::error_code& ec);

    uint32_t OutputSize() const
    {
        return m_truncSize;
    }

private:
    static IHMACAlgo& GetHMAC(ICryptoProvider& provider, HMACMode mode);

    HMACMode m_mode;
    IHMACAlgo* m_pHMAC;
    uint32_t m_truncSize;
    std::array<uint8_t, AuthSizes::MAX_HMAC_OUTPUT_SIZE> m_buffer;
};

} // namespace opendnp3

#endif
