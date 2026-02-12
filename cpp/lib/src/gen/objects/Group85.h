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

#ifndef OPENDNP3_GROUP85_H
#define OPENDNP3_GROUP85_H

#include "opendnp3/app/GroupVariationID.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <array>
#include <cstdint>
#include <vector>

namespace opendnp3
{

/// Data Set Prototype - With UUID (Group 85 Var 1)
///
/// IEEE 1815-2012 Section 11.8.1
/// Used to define the structure of a data set. Contains a UUID followed by
/// a sequence of element descriptors that define the prototype's elements.
///
/// Wire format (free-format, qualifier 0x5B):
///   UUID (16 bytes) + element descriptors (variable)
///
/// Each element descriptor:
///   descriptor_type (1 byte) + data_type (1 byte) + max_length (1 byte) + name (variable)
struct Group85Var1
{
    static GroupVariationID ID()
    {
        return GroupVariationID(85, 1);
    }

    /// The 16-byte UUID identifying this prototype
    std::array<uint8_t, 16> uuid;

    /// Raw element descriptor data following the UUID
    std::vector<uint8_t> elements;

    /// Total size of this object on the wire
    size_t Size() const
    {
        return 16 + elements.size();
    }

    /// Read from a buffer
    bool Read(const ser4cpp::rseq_t& buffer)
    {
        if (buffer.length() < 16)
        {
            return false;
        }

        std::copy(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + 16, uuid.begin());

        if (buffer.length() > 16)
        {
            elements.assign(static_cast<const uint8_t*>(buffer) + 16,
                            static_cast<const uint8_t*>(buffer) + buffer.length());
        }
        else
        {
            elements.clear();
        }

        return true;
    }
};

} // namespace opendnp3

#endif
