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
#include "secauth/outstation/RoleBasedPermissions.h"

namespace opendnp3
{

const Permissions RoleBasedPermissions::OPERATE_CONTROLS(Permissions::Allowed(FunctionCode::SELECT,
                                                                              FunctionCode::OPERATE,
                                                                              FunctionCode::DIRECT_OPERATE));

const Permissions RoleBasedPermissions::MONITOR_DATA(Permissions::Allowed(FunctionCode::READ));

Permissions RoleBasedPermissions::From(UserRole role)
{
    switch (role)
    {
    case (UserRole::VIEWER):
        return MONITOR_DATA;
    case (UserRole::OPERATOR):
        return MONITOR_DATA | OPERATE_CONTROLS;
    case (UserRole::SINGLE_USER):
        return Permissions::AllowAll();
    default:
        return Permissions::AllowNothing();
    }
}

} // namespace opendnp3
