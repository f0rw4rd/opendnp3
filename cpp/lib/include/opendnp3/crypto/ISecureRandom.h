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
#ifndef OPENDNP3_ISECURERANDOM_H
#define OPENDNP3_ISECURERANDOM_H

#include <ser4cpp/container/SequenceTypes.h>

#include <system_error>

namespace opendnp3
{

/**
 * Interface for generating cryptographically secure random bytes.
 * Implementations must be thread-safe.
 */
class ISecureRandom
{
public:
    virtual ~ISecureRandom() {}

    /**
     * Fill the destination buffer with secure random bytes.
     * @param dest Buffer to fill with random bytes
     * @param ec Set on failure
     * @return Read-only view of the generated random bytes
     */
    virtual ser4cpp::rseq_t GetSecureRandom(ser4cpp::wseq_t& dest, std::error_code& ec) = 0;
};

} // namespace opendnp3

#endif
