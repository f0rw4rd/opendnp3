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
#include "secauth/Crypto.h"

namespace opendnp3
{

bool Crypto::TryGetHMACMode(HMACType type, HMACMode& mode)
{
    switch (type)
    {
    case (HMACType::HMAC_SHA1_TRUNC_10):
        mode = HMACMode::SHA1_TRUNC_10;
        return true;
    case (HMACType::HMAC_SHA1_TRUNC_8):
        mode = HMACMode::SHA1_TRUNC_8;
        return true;
    case (HMACType::HMAC_SHA256_TRUNC_16):
        mode = HMACMode::SHA256_TRUNC_16;
        return true;
    case (HMACType::HMAC_SHA256_TRUNC_8):
        mode = HMACMode::SHA256_TRUNC_8;
        return true;
    default:
        return false;
    }
}

} // namespace opendnp3
