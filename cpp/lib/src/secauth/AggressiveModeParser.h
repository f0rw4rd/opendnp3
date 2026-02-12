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
#ifndef OPENDNP3_AGGRESSIVEMODEPARSER_H
#define OPENDNP3_AGGRESSIVEMODEPARSER_H

#include "app/parsing/ParseResult.h"
#include "gen/objects/Group120.h"

#include "opendnp3/logging/Logger.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>

namespace opendnp3
{

/**
 * Result of checking if an APDU uses aggressive mode.
 */
struct AggModeResult
{
    /// failure constructor
    explicit AggModeResult(ParseResult result_);

    /// success constructor
    AggModeResult(const Group120Var3& request, const ser4cpp::rseq_t& remainder);

    ParseResult result;
    bool isAggMode;
    Group120Var3 request;
    ser4cpp::rseq_t remainder;

    AggModeResult() = delete;
};

/**
 * Result of parsing the aggressive mode HMAC trailer.
 */
struct AggModeHMACResult
{
    /// failure constructor
    explicit AggModeHMACResult(ParseResult result_);

    /// success constructor
    AggModeHMACResult(const Group120Var9& hmac, const ser4cpp::rseq_t& objects);

    ParseResult result;
    Group120Var9 hmac;
    ser4cpp::rseq_t objects;

    AggModeHMACResult() = delete;
};

/**
 * Static parser for aggressive mode authentication.
 * Checks if an APDU's object headers start with a Group120Var3 (aggressive mode request),
 * and if so, extracts it along with the trailing Group120Var9 HMAC.
 */
struct AggressiveModeParser
{
    AggressiveModeParser() = delete;

    /// Check if the objects begin with a Group120Var3 aggressive mode request
    static AggModeResult IsAggressiveMode(ser4cpp::rseq_t objects, Logger* pLogger);

    /// Parse the trailing Group120Var9 HMAC from the remainder
    static AggModeHMACResult ParseHMAC(ser4cpp::rseq_t remainder, uint32_t hmacSize, Logger* pLogger);
};

} // namespace opendnp3

#endif
