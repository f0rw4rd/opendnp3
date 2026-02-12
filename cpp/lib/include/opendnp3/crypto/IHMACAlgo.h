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
#ifndef OPENDNP3_IHMACALGO_H
#define OPENDNP3_IHMACALGO_H

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>
#include <initializer_list>
#include <system_error>

namespace opendnp3
{

/**
 * Abstract interface for an HMAC algorithm (e.g. HMAC-SHA-1, HMAC-SHA-256).
 */
class IHMACAlgo
{
public:
    virtual ~IHMACAlgo() {}

    /// The output size (in bytes) of the HMAC algorithm
    virtual uint16_t OutputSize() const = 0;

    /**
     * Calculate the HMAC value over one or more data segments.
     * @param key The HMAC key
     * @param data One or more data segments to authenticate
     * @param dest Output buffer (must be >= OutputSize() bytes)
     * @param ec Set on failure
     * @return Read-only view of the computed HMAC value
     */
    virtual ser4cpp::rseq_t Calculate(const ser4cpp::rseq_t& key,
                                      std::initializer_list<ser4cpp::rseq_t> data,
                                      ser4cpp::wseq_t& dest,
                                      std::error_code& ec)
        = 0;
};

} // namespace opendnp3

#endif
