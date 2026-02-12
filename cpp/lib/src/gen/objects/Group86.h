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

#ifndef OPENDNP3_GROUP86_H
#define OPENDNP3_GROUP86_H

#include "opendnp3/app/GroupVariationID.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>
#include <vector>

namespace opendnp3
{

/// Data Set Descriptor - Any Variation (Group 86 Var 0)
struct Group86Var0
{
    static GroupVariationID ID()
    {
        return GroupVariationID(86, 0);
    }
};

/// Data Set Descriptor - Contents (Group 86 Var 1)
///
/// IEEE 1815-2012 Section 11.8.2
/// Describes the contents of a data set. Each element descriptor specifies
/// a data type code, max length, and an element name string.
///
/// Wire format (free-format):
///   element_count (variable) + element descriptors
///
/// Each element descriptor:
///   data_type (1 byte) + max_length (1 byte) + name (variable, null-terminated)
struct Group86Var1
{
    static GroupVariationID ID()
    {
        return GroupVariationID(86, 1);
    }

    /// Raw descriptor data
    std::vector<uint8_t> data;

    size_t Size() const
    {
        return data.size();
    }

    bool Read(const ser4cpp::rseq_t& buffer)
    {
        data.assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + buffer.length());
        return true;
    }
};

/// Data Set Descriptor - Characteristics (Group 86 Var 2)
///
/// IEEE 1815-2012 Section 11.8.2
/// Defines characteristics of data set elements such as whether they are
/// mandatory or optional, and whether the outstation owns the element.
///
/// Wire format (free-format):
///   Per element: characteristics_byte (1 byte)
struct Group86Var2
{
    static GroupVariationID ID()
    {
        return GroupVariationID(86, 2);
    }

    /// Raw characteristics data
    std::vector<uint8_t> data;

    size_t Size() const
    {
        return data.size();
    }

    bool Read(const ser4cpp::rseq_t& buffer)
    {
        data.assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + buffer.length());
        return true;
    }
};

/// Data Set Descriptor - Point Index Attributes (Group 86 Var 3)
///
/// IEEE 1815-2012 Section 11.8.2
/// Maps data set elements to outstation point indices, linking each
/// element to its corresponding data type group and point index.
///
/// Wire format (free-format):
///   Per element: point_type (1 byte) + point_index (2 bytes)
struct Group86Var3
{
    static GroupVariationID ID()
    {
        return GroupVariationID(86, 3);
    }

    /// Raw point index attribute data
    std::vector<uint8_t> data;

    size_t Size() const
    {
        return data.size();
    }

    bool Read(const ser4cpp::rseq_t& buffer)
    {
        data.assign(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + buffer.length());
        return true;
    }
};

} // namespace opendnp3

#endif
