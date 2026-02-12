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
#ifndef OPENDNP3_AUTHCONSTANTS_H
#define OPENDNP3_AUTHCONSTANTS_H

#include "opendnp3/gen/SecurityStatIndex.h"
#include "opendnp3/util/StaticOnly.h"

#include <cstdint>

namespace opendnp3
{

struct AuthConstants : StaticOnly
{
    /// Default maximum number of authenticated messages before session key change
    static const uint16_t DEFAULT_SESSION_KEY_MAX_AUTH_COUNT = 1000;

    /// Default session key change interval in minutes
    static const uint8_t DEFAULT_SESSION_KEY_CHANGE_MINUTES = 15;
};

} // namespace opendnp3

#endif
