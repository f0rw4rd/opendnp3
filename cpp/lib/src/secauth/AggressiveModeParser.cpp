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
#include "secauth/AggressiveModeParser.h"

#include "app/GroupVariationRecord.h"
#include "app/parsing/ObjectHeaderParser.h"
#include "logging/LogMacros.h"

#include "opendnp3/logging/LogLevels.h"

#include <ser4cpp/serialization/LittleEndian.h>

namespace opendnp3
{

AggModeResult::AggModeResult(ParseResult result_) : result(result_), isAggMode(false) {}

AggModeResult::AggModeResult(const Group120Var3& request_, const ser4cpp::rseq_t& remainder_)
    : result(ParseResult::OK), isAggMode(true), request(request_), remainder(remainder_)
{
}

AggModeHMACResult::AggModeHMACResult(ParseResult result_) : result(result_) {}

AggModeHMACResult::AggModeHMACResult(const Group120Var9& hmac_, const ser4cpp::rseq_t& objects_)
    : result(ParseResult::OK), hmac(hmac_), objects(objects_)
{
}

AggModeResult AggressiveModeParser::IsAggressiveMode(ser4cpp::rseq_t objects, Logger* pLogger)
{
    if (objects.length() == 0)
    {
        return AggModeResult(ParseResult::OK);
    }

    ObjectHeader header;
    auto result = ObjectHeaderParser::ParseObjectHeader(header, objects, pLogger);
    if (result != ParseResult::OK)
    {
        return AggModeResult(result);
    }

    auto record = GroupVariationRecord::GetRecord(header.group, header.variation);

    if (record.enumeration != GroupVariation::Group120Var3)
    {
        return AggModeResult(result);
    }

    if (QualifierCodeSpec::from_type(header.qualifier) != QualifierCode::UINT8_CNT)
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::WARN, "Aggressive mode request contains bad qualifier (%u)",
                            header.qualifier);
        return AggModeResult(ParseResult::INVALID_OBJECT_QUALIFIER);
    }

    if (objects.length() < 1)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Insufficient data for count");
        return AggModeResult(ParseResult::NOT_ENOUGH_DATA_FOR_HEADER);
    }

    uint8_t count = 0;
    ser4cpp::UInt8::read_from(objects, count);

    if (count != 1)
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::WARN, "Aggressive mode request contains bad count (%u)", count);
        return AggModeResult(ParseResult::NOT_ON_WHITELIST);
    }

    Group120Var3 value;
    if (Group120Var3::Read(objects, value))
    {
        return AggModeResult(value, objects);
    }
    else
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Insufficient data for g120v3");
        return AggModeResult(ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS);
    }
}

AggModeHMACResult AggressiveModeParser::ParseHMAC(ser4cpp::rseq_t remainder, uint32_t hmacSize, Logger* pLogger)
{
    // The trailer format is: group(1) + var(1) + qualifier(1) + count(1) + size(2) + HMAC(hmacSize)
    const uint32_t TRAILER_SIZE = 6 + hmacSize;
    if (remainder.length() < TRAILER_SIZE)
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::WARN,
                            "Not enough data for aggressive mode hmac with expected length of (%u)", hmacSize);
        return AggModeHMACResult(ParseResult::NOT_ENOUGH_DATA_FOR_HEADER);
    }

    const uint32_t OBJECTS_SIZE = static_cast<uint32_t>(remainder.length()) - TRAILER_SIZE;

    auto objects = remainder.take(OBJECTS_SIZE);
    auto trailer = remainder.skip(OBJECTS_SIZE);

    ObjectHeader header;
    auto result = ObjectHeaderParser::ParseObjectHeader(header, trailer, pLogger);
    if (result != ParseResult::OK)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for aggressive mode hmac header");
        return AggModeHMACResult(result);
    }

    auto record = GroupVariationRecord::GetRecord(header.group, header.variation);
    if (record.enumeration != GroupVariation::Group120Var9)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN,
                            "Aggressive mode request doesn't contain g120v9 at expected position");
        return AggModeHMACResult(ParseResult::UNKNOWN_OBJECT);
    }

    if (QualifierCodeSpec::from_type(header.qualifier) != QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT)
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::WARN, "Aggressive mode hmac contains unexpected qualifier (%u)",
                            header.qualifier);
        return AggModeHMACResult(ParseResult::UNKNOWN_QUALIFIER);
    }

    uint8_t count = 0;
    uint16_t size = 0;
    if (!ser4cpp::UInt8::read_from(trailer, count) || !ser4cpp::UInt16::read_from(trailer, size))
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format count and/or size");
        return AggModeHMACResult(ParseResult::NOT_ENOUGH_DATA_FOR_HEADER);
    }

    if (size != trailer.length())
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Agg mode free-format header doesn't contain expected data");
        return AggModeHMACResult(ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS);
    }

    if (size == hmacSize)
    {
        Group120Var9 hmac(trailer);
        return AggModeHMACResult(hmac, objects);
    }
    else
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::WARN, "Actual length of hmac (%u) doesn't match expected length of (%u)",
                            size, hmacSize);
        return AggModeHMACResult(ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS);
    }
}

} // namespace opendnp3
