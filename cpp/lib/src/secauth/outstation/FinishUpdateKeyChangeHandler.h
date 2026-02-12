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
#ifndef OPENDNP3_FINISHUPDATEKEYCHANGEHANDLER_H
#define OPENDNP3_FINISHUPDATEKEYCHANGEHANDLER_H

#include "app/parsing/IAPDUHandler.h"
#include "gen/objects/Group120.h"

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

/**
 * APDU handler for parsing the "Finish Update Key Change" request.
 * Expects exactly two object headers:
 *   1. Group120Var13 (Encrypted Update Key Data)
 *   2. Group120Var14 (Digital Signature) OR Group120Var15 (HMAC)
 */
class FinishUpdateKeyChangeHandler final : public IAPDUHandler
{
public:
    enum Result : uint8_t
    {
        NONE,             // if the required objects were not present
        HMAC,             // if the 2nd object was a g120v15
        DIGITAL_SIGNATURE // if the 2nd object was a g120v14
    };

    FinishUpdateKeyChangeHandler() : m_result(Result::NONE) {}

    Result GetResult() const
    {
        return m_result;
    }

    virtual bool IsAllowed(uint32_t headerCount, GroupVariation gv, QualifierCode qc) override;

    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var13& value,
                                   const ser4cpp::rseq_t& object) override;
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var14& value,
                                   const ser4cpp::rseq_t& object) override;
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var15& value,
                                   const ser4cpp::rseq_t& object) override;

    /// The actual data that will be set correctly if the result != NONE
    Group120Var13 keyChange;
    ser4cpp::rseq_t authData;

private:
    Result m_result;
};

} // namespace opendnp3

#endif
