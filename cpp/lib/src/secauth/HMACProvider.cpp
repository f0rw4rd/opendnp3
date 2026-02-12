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
#include "secauth/HMACProvider.h"

namespace opendnp3
{

IHMACAlgo& HMACProvider::GetHMAC(ICryptoProvider& crypto, HMACMode mode)
{
    switch (mode)
    {
    case (HMACMode::SHA1_TRUNC_8):
    case (HMACMode::SHA1_TRUNC_10):
        return crypto.GetSHA1HMAC();
    default:
        return crypto.GetSHA256HMAC();
    }
}

HMACProvider::HMACProvider(ICryptoProvider& crypto, HMACMode mode)
    : m_mode(mode), m_pHMAC(&GetHMAC(crypto, mode)), m_truncSize(GetTruncationSize(mode))
{
    m_buffer.fill(0);
}

HMACType HMACProvider::GetType() const
{
    return ToHMACType(m_mode);
}

ser4cpp::rseq_t HMACProvider::Compute(const ser4cpp::rseq_t& key,
                                      std::initializer_list<ser4cpp::rseq_t> buffers,
                                      std::error_code& ec)
{
    ser4cpp::wseq_t dest(m_buffer.data(), static_cast<uint32_t>(m_buffer.size()));
    auto result = m_pHMAC->Calculate(key, buffers, dest, ec);

    if (ec)
    {
        return ser4cpp::rseq_t();
    }

    return result.take(m_truncSize);
}

} // namespace opendnp3
