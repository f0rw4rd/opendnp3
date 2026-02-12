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
#ifndef OPENDNP3_CRITICALFUNCTIONS_H
#define OPENDNP3_CRITICALFUNCTIONS_H

#include "opendnp3/gen/FunctionCode.h"

namespace opendnp3
{

/**
 * Configures which DNP3 function codes require authentication.
 * Some function codes are always critical (e.g. WRITE, SELECT, OPERATE).
 * Others are optionally critical depending on the security policy.
 */
class CriticalFunctions
{
public:
    /// Factory: optional codes do NOT require authentication
    static CriticalFunctions AuthOptional();

    /// Factory: ALL function codes require authentication
    static CriticalFunctions AuthEverything();

    bool authConfirm;
    bool authRead;
    bool authImmediateFreeze;
    bool authImmediateFreezeNR;
    bool authFreezeClear;
    bool authFreezeClearNR;
    bool authFreezeAtTime;
    bool authFreezeAtTimeNR;
    bool authInitData;
    bool authAssignClass;
    bool authDelayMeasure;
    bool authResponse;
    bool authUnsolicited;

    /**
     * Determine if a given function code requires authentication.
     * Always-critical codes (WRITE, SELECT, OPERATE, etc.) always return true.
     * Optional codes return based on the configuration.
     */
    bool IsCritical(FunctionCode code) const;

private:
    CriticalFunctions() = delete;
    explicit CriticalFunctions(bool authOptionalCodes);
};

} // namespace opendnp3

#endif
