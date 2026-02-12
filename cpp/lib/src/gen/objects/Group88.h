/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Created 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Step Function I/O
 * LLC (https://stepfunc.io) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Step Function I/O LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPENDNP3_GROUP88_H
#define OPENDNP3_GROUP88_H

#include "opendnp3/app/GroupVariationID.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>
#include <vector>

namespace opendnp3
{

/// Data Set Snapshot - Event (Group 88 Var 1)
///
/// IEEE 1815-2012 Section 11.8.4
/// Contains a snapshot of a data set's values at a specific point in time,
/// used for event reporting. Includes a timestamp followed by element values.
///
/// Wire format (free-format or indexed):
///   timestamp (6 bytes, DNP3 48-bit time) + element values
///
/// Element values are encoded same as Group 87 Var 1.
struct Group88Var1
{
    static GroupVariationID ID()
    {
        return GroupVariationID(88, 1);
    }

    /// DNP3 timestamp (milliseconds since epoch, 48-bit)
    uint64_t timestamp = 0;

    /// Raw element value data following the timestamp
    std::vector<uint8_t> data;

    size_t Size() const
    {
        return 6 + data.size();
    }

    bool Read(const ser4cpp::rseq_t& buffer)
    {
        if (buffer.length() < 6)
        {
            return false;
        }

        timestamp = 0;
        for (int j = 0; j < 6; ++j)
        {
            timestamp |= static_cast<uint64_t>(buffer[j]) << (j * 8);
        }

        if (buffer.length() > 6)
        {
            data.assign(static_cast<const uint8_t*>(buffer) + 6, static_cast<const uint8_t*>(buffer) + buffer.length());
        }
        else
        {
            data.clear();
        }

        return true;
    }
};

} // namespace opendnp3

#endif
