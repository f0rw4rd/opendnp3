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
#ifndef OPENDNP3_AUTHORITYKEY_H
#define OPENDNP3_AUTHORITYKEY_H

#include <ser4cpp/container/SequenceTypes.h>

#include <array>
#include <cstdint>

namespace opendnp3
{

/**
 * Container for an Authority Symmetric Key.
 * Used during update key change procedures.
 * Maximum size is 32 bytes (256 bits).
 */
class AuthorityKey
{
public:
    static const uint8_t MAX_KEY_SIZE = 32;

    AuthorityKey();
    explicit AuthorityKey(const ser4cpp::rseq_t& key);

    ser4cpp::rseq_t GetView() const;
    bool IsValid() const;
    bool Initialize(const ser4cpp::rseq_t& key);

private:
    std::array<uint8_t, MAX_KEY_SIZE> m_buffer;
    uint8_t m_size;
};

} // namespace opendnp3

#endif
