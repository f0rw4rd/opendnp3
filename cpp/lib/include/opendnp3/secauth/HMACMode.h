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
#ifndef OPENDNP3_HMACMODE_H
#define OPENDNP3_HMACMODE_H

#include "opendnp3/gen/HMACType.h"

#include <cstdint>

namespace opendnp3
{

/**
 * Specifies the configured HMAC mode.
 * This is a subset of HMACType values that are valid for SA5 configuration.
 */
enum class HMACMode : uint8_t
{
    SHA1_TRUNC_10,
    SHA1_TRUNC_8,
    SHA256_TRUNC_8,
    SHA256_TRUNC_16
};

/// Convert HMACMode to the wire-level HMACType enum
HMACType ToHMACType(HMACMode mode);

/// Get the truncation size in bytes for a given HMAC mode
uint32_t GetTruncationSize(HMACMode mode);

} // namespace opendnp3

#endif
