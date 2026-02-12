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
#ifndef OPENDNP3_SESSIONKEYSVIEW_H
#define OPENDNP3_SESSIONKEYSVIEW_H

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

/**
 * Read-only view of a session key pair (control direction and monitoring direction).
 * CDSK = Control Direction Session Key (master -> outstation commands)
 * MDSK = Monitoring Direction Session Key (outstation -> master responses)
 */
struct SessionKeysView
{
    SessionKeysView() {}

    SessionKeysView(const ser4cpp::rseq_t& controlKey_, const ser4cpp::rseq_t& monitorKey_)
        : controlKey(controlKey_), monitorKey(monitorKey_)
    {
    }

    bool IsValid() const
    {
        return controlKey.is_not_empty() && monitorKey.is_not_empty();
    }

    ser4cpp::rseq_t controlKey;
    ser4cpp::rseq_t monitorKey;
};

} // namespace opendnp3

#endif
