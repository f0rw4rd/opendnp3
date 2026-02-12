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
#ifndef OPENDNP3_STATISTICS_H
#define OPENDNP3_STATISTICS_H

#include "opendnp3/gen/SecurityStatIndex.h"
#include "opendnp3/secauth/AuthConstants.h"
#include "opendnp3/util/Uncopyable.h"

#include <cstdint>

namespace opendnp3
{

/**
 * Class for tracking security statistics as defined in IEEE 1815-2012.
 */
struct Statistics : private Uncopyable
{
public:
    Statistics();

    /// Increment and return the value of the specified stat
    uint32_t Increment(SecurityStatIndex index);

    /// Get the value of the specified stat
    uint32_t GetValue(SecurityStatIndex index) const;

private:
    uint32_t statistics[NUM_SECURITY_STATS];
};

} // namespace opendnp3

#endif
