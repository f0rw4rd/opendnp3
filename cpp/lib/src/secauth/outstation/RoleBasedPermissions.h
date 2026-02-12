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
#ifndef OPENDNP3_ROLEBASEDPERMISSIONS_H
#define OPENDNP3_ROLEBASEDPERMISSIONS_H

#include "secauth/outstation/Permissions.h"

#include "opendnp3/gen/UserRole.h"
#include "opendnp3/util/StaticOnly.h"

namespace opendnp3
{

/**
 * Maps user roles to permission sets.
 * Currently VIEWER, OPERATOR, and SINGLE_USER roles have permissions mapped.
 */
class RoleBasedPermissions : StaticOnly
{
public:
    static Permissions From(UserRole role);

private:
    static const Permissions OPERATE_CONTROLS;
    static const Permissions MONITOR_DATA;
};

} // namespace opendnp3

#endif
