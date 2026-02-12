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
#ifndef OPENDNP3_STATTHRESHOLDS_H
#define OPENDNP3_STATTHRESHOLDS_H

#include "opendnp3/gen/SecurityStatIndex.h"

#include <cstdint>

namespace opendnp3
{

/**
 * Deadband thresholds for the 18 SA security statistics.
 * When a statistic changes by more than its deadband, an event is generated.
 */
class StatThresholds
{
public:
    StatThresholds();

    /// Get the deadband for a given stat index. Returns UINT32_MAX if out of range.
    uint32_t GetDeadband(uint16_t index) const;

    /// Set the deadband for a specific security statistic
    void Set(SecurityStatIndex index, uint32_t threshold);

private:
    uint32_t thresholds[NUM_SECURITY_STATS];

    static const uint32_t DEFAULTS[NUM_SECURITY_STATS];
};

} // namespace opendnp3

#endif
