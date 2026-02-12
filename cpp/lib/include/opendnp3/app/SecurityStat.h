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
#ifndef OPENDNP3_SECURITYSTAT_H
#define OPENDNP3_SECURITYSTAT_H

#include "opendnp3/app/DNPTime.h"

#include <cstdint>

namespace opendnp3
{

/**
    SA security statistic object as used by the API.
*/
class SecurityStat
{
public:
    // this is the easiest way to make the SecurityStats look like other types
    struct Value
    {
        uint16_t assocId;
        uint32_t count;
    };

    SecurityStat();

    SecurityStat(Value value, uint8_t quality, DNPTime time);

    SecurityStat(uint8_t quality, uint16_t assocId, uint32_t count);

    SecurityStat(uint8_t quality, uint16_t assocId, uint32_t count, DNPTime time);

    uint8_t quality; //	bitfield that stores type specific quality flags
    Value value;     //	assocId and count
    DNPTime time;    //	timestamp associated with the measurement (may not be set)
};

} // namespace opendnp3

#endif
