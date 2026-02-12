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
#ifndef OPENDNP3_SINGLEVALUEHANDLER_H
#define OPENDNP3_SINGLEVALUEHANDLER_H

#include "app/parsing/IAPDUHandler.h"

#include "opendnp3/gen/GroupVariation.h"
#include "opendnp3/gen/QualifierCode.h"
#include "opendnp3/util/Uncopyable.h"

namespace opendnp3
{

/**
 * Template handler that expects exactly one object header matching a specific
 * GroupVariation and QualifierCode. Used for parsing single SA auth objects
 * from responses or requests.
 */
template<class ValueType, GroupVariation GV, QualifierCode QC>
class SingleValueHandler : public IAPDUHandler, private Uncopyable
{
public:
    SingleValueHandler() : m_valid(false) {}

    ValueType value;

    bool IsValid() const
    {
        return m_valid;
    }

    virtual bool IsAllowed(uint32_t headerCount, GroupVariation gv, QualifierCode qc) override final
    {
        return (headerCount == 0) && (gv == GV) && (qc == QC);
    }

protected:
    bool m_valid;
};

} // namespace opendnp3

#endif
