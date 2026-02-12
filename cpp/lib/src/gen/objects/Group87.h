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

#ifndef OPENDNP3_GROUP87_H
#define OPENDNP3_GROUP87_H

#include "opendnp3/app/GroupVariationID.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>
#include <vector>

namespace opendnp3
{

/// Data Set Present Value (Group 87 Var 1)
///
/// IEEE 1815-2012 Section 11.8.3
/// Contains the current (present) value of all elements in a data set.
/// The element values are encoded according to the data types defined
/// in the corresponding descriptor (Group 86).
///
/// Wire format (free-format or indexed):
///   Per element: length (1 byte) + value (length bytes)
///
/// The data types and ordering are defined by the descriptor.
struct Group87Var1
{
    static GroupVariationID ID()
    {
        return GroupVariationID(87, 1);
    }

    /// Raw element value data
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
