/*
 * Copyright 2013-2022 Step Function I/O, LLC
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
#ifndef OPENDNP3_DEVICEATTRIBUTES_H
#define OPENDNP3_DEVICEATTRIBUTES_H

#include <cstdint>
#include <string>
#include <vector>

namespace opendnp3
{

/// Type codes for device attribute data (IEEE 1815-2012 Table 6-14)
enum class DeviceAttrType : uint8_t
{
    VISIBLE_STRING = 1,
    UNSIGNED_INT = 2,
    SIGNED_INT = 3,
    FLOATING_POINT = 4,
    OCTET_STRING = 5,
    BIT_STRING = 6,
    DNP3_TIME = 7,
    UNICODE = 8,
    ATTR_LIST = 254,
    EXT_ATTR_LIST = 255
};

/// A variant-like storage for decoded device attribute values.
struct DeviceAttributeValue
{
    DeviceAttrType type = DeviceAttrType::VISIBLE_STRING;

    /// For VISIBLE_STRING
    std::string stringValue;

    /// For UNSIGNED_INT
    uint32_t unsignedValue = 0;

    /// For SIGNED_INT
    int32_t signedValue = 0;

    /// For FLOATING_POINT (both float32 and float64)
    double floatValue = 0.0;

    /// For DNP3_TIME (milliseconds since epoch)
    uint64_t timeValue = 0;

    /// For OCTET_STRING, BIT_STRING, ATTR_LIST, EXT_ATTR_LIST
    std::vector<uint8_t> rawValue;
};

} // namespace opendnp3

#endif
