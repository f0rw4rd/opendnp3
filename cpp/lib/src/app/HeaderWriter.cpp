/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Modified 2024-2026 f0rw4rd (experimental fork)
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
#include "HeaderWriter.h"

#include <ser4cpp/serialization/LittleEndian.h>

#include <cassert>
#include <cstring>

namespace opendnp3
{

HeaderWriter::HeaderWriter(ser4cpp::wseq_t* position_) : position(position_) {}

size_t HeaderWriter::Remaining() const
{
    return position->length();
}

void HeaderWriter::Mark()
{
    mark.set(*position);
}

bool HeaderWriter::Rollback()
{
    if (mark.is_set())
    {
        *position = mark.get();
        mark.clear();
        return true;
    }

    return false;
}

bool HeaderWriter::WriteRawBytes(const uint8_t* data, size_t length)
{
    if (position->length() < length)
        return false;
    memcpy(static_cast<uint8_t*>(*position), data, length);
    position->advance(length);
    return true;
}

bool HeaderWriter::WriteHeader(GroupVariationID id, QualifierCode qc)
{
    if (position->length() < 3)
    {
        return false;
    }

    ser4cpp::UInt8::write_to(*position, id.group);
    ser4cpp::UInt8::write_to(*position, id.variation);
    ser4cpp::UInt8::write_to(*position, QualifierCodeSpec::to_type(qc));
    return true;
}

bool HeaderWriter::WriteHeaderWithReserve(GroupVariationID id, QualifierCode qc, size_t reserve)
{
    return (position->length() < (3 + reserve)) ? false : WriteHeader(id, qc);
}

bool HeaderWriter::WriteFreeFormat(const IVariableLength& value)
{
    const auto objectSize = value.Size();
    // header(3) + count(1) + size(2) + objectData(N)
    const size_t totalNeeded = 3 + 1 + 2 + objectSize;

    if (position->length() < totalNeeded)
    {
        return false;
    }

    auto gvid = value.InstanceID();

    // Write the header: group, variation, qualifier 0x5B
    ser4cpp::UInt8::write_to(*position, gvid.group);
    ser4cpp::UInt8::write_to(*position, gvid.variation);
    ser4cpp::UInt8::write_to(*position, QualifierCodeSpec::to_type(QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT));

    // Write count = 1
    ser4cpp::UInt8::write_to(*position, 1);

    // Write the size prefix
    ser4cpp::UInt16::write_to(*position, static_cast<uint16_t>(objectSize));

    // Write the object data
    return value.Write(*position);
}

} // namespace opendnp3
