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
#include "secauth/AuthorityKey.h"

#include <algorithm>
#include <cstring>

namespace opendnp3
{

AuthorityKey::AuthorityKey() : m_size(0)
{
    m_buffer.fill(0);
}

AuthorityKey::AuthorityKey(const ser4cpp::rseq_t& key) : m_size(0)
{
    m_buffer.fill(0);
    Initialize(key);
}

ser4cpp::rseq_t AuthorityKey::GetView() const
{
    return ser4cpp::rseq_t(m_buffer.data(), m_size);
}

bool AuthorityKey::IsValid() const
{
    return m_size > 0;
}

bool AuthorityKey::Initialize(const ser4cpp::rseq_t& key)
{
    if (key.length() == 0 || key.length() > MAX_KEY_SIZE)
    {
        m_size = 0;
        return false;
    }
    m_size = static_cast<uint8_t>(key.length());
    std::memcpy(m_buffer.data(), key, m_size);
    return true;
}

} // namespace opendnp3
