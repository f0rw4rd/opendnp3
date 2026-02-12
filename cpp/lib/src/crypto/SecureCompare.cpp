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
#include "opendnp3/crypto/SecureCompare.h"

namespace opendnp3
{

bool SecureEquals(const ser4cpp::rseq_t& lhs, const ser4cpp::rseq_t& rhs)
{
    if (lhs.length() != rhs.length())
    {
        return false;
    }

    uint8_t result = 0;

    // Accumulate all different bits in each byte position.
    // This avoids early-exit and runs in constant time for equal-length inputs.
    for (size_t i = 0; i < lhs.length(); ++i)
    {
        result |= lhs[i] ^ rhs[i];
    }

    return result == 0;
}

} // namespace opendnp3
