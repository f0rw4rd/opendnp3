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
#ifndef OPENDNP3_USERROLE_H
#define OPENDNP3_USERROLE_H

#include <cstdint>

namespace opendnp3
{

/**
 * Pre-defined user roles in Secure Authentication v5.
 */
enum class UserRole : uint16_t
{
    VIEWER = 0,
    OPERATOR = 1,
    ENGINEER = 2,
    INSTALLER = 3,
    SECADM = 4,
    SECAUD = 5,
    RBACMNT = 6,
    SINGLE_USER = 32768,
    UNDEFINED = 32767
};

} // namespace opendnp3

#endif
