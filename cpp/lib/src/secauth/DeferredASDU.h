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
#ifndef OPENDNP3_DEFERREDASDU_H
#define OPENDNP3_DEFERREDASDU_H

#include "opendnp3/app/AppConstants.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <array>
#include <cstdint>

namespace opendnp3
{

/**
 * Stores a deferred ASDU (Application Service Data Unit) while waiting
 * for authentication to complete. This holds a critical APDU that was
 * received but could not be processed until the challenge-reply handshake
 * succeeds.
 */
class DeferredASDU
{
public:
    DeferredASDU() : m_size(0), m_isSet(false) {}

    /// Store a copy of the ASDU data
    bool Set(const ser4cpp::rseq_t& asdu);

    /// Retrieve the stored ASDU as a read-only sequence
    ser4cpp::rseq_t Get() const;

    /// Check if an ASDU is stored
    bool IsSet() const
    {
        return m_isSet;
    }

    /// Clear the stored ASDU
    void Clear()
    {
        m_isSet = false;
        m_size = 0;
    }

private:
    static const uint32_t MAX_ASDU_SIZE = DEFAULT_MAX_APDU_SIZE;

    std::array<uint8_t, DEFAULT_MAX_APDU_SIZE> m_buffer;
    uint32_t m_size;
    bool m_isSet;
};

} // namespace opendnp3

#endif
