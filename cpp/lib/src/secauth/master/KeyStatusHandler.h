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
#ifndef OPENDNP3_KEYSTATUSHANDLER_H
#define OPENDNP3_KEYSTATUSHANDLER_H

#include "app/parsing/IAPDUHandler.h"
#include "gen/objects/Group120.h"

#include "opendnp3/logging/Logger.h"

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

/**
 * APDU handler that parses a single Group120Var5 (Key Status) response.
 * Used by the master to read the outstation's key status during a session key exchange.
 */
class KeyStatusHandler : public IAPDUHandler
{
public:
    explicit KeyStatusHandler(Logger logger);

    virtual bool IsAllowed(uint32_t headerCount, GroupVariation gv, QualifierCode) override final
    {
        return (headerCount == 0) && (gv == GroupVariation::Group120Var5);
    }

    bool GetStatus(Group120Var5& status, ser4cpp::rseq_t& rawObject) const;

private:
    Logger logger;
    bool valid;
    Group120Var5 status;
    ser4cpp::rseq_t rawObject;

    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var5& status,
                                   const ser4cpp::rseq_t& rawObject) override final;
};

} // namespace opendnp3

#endif
