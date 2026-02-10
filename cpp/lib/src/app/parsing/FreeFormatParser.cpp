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
#include "FreeFormatParser.h"

#include "app/parsing/BufferedCollection.h"
#include "logging/LogMacros.h"

#include "opendnp3/app/DeviceAttributes.h"
#include "opendnp3/app/OctetString.h"
#include "opendnp3/logging/LogLevels.h"

#include <ser4cpp/serialization/LittleEndian.h>

#include <algorithm>
#include <cstring>

namespace opendnp3
{

ParseResult FreeFormatParser::ParseHeader(ser4cpp::rseq_t& buffer,
                                          const ParserSettings& settings,
                                          const HeaderRecord& record,
                                          Logger* pLogger,
                                          IAPDUHandler* pHandler)
{
    // Free-format qualifier 0x5B: 1-byte count, then per object: 2-byte length + data
    if (buffer.length() < 1)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format count");
        return ParseResult::NOT_ENOUGH_DATA_FOR_HEADER;
    }

    uint8_t count = buffer[0];
    buffer.advance(1);

    FORMAT_LOGGER_BLOCK(pLogger, settings.LoggingLevel(), "%03u,%03u %s, %s [%u]", record.group, record.variation,
                        GroupVariationSpec::to_human_string(record.enumeration),
                        QualifierCodeSpec::to_human_string(record.GetQualifierCode()), count);

    if (settings.ExpectsContents())
    {
        return ParseFreeFormatObjects(buffer, record, count, pLogger, pHandler);
    }

    if (pHandler)
    {
        pHandler->OnHeader(CountHeader(record, count));
    }

    return ParseResult::OK;
}

ParseResult FreeFormatParser::ParseFreeFormatObjects(
    ser4cpp::rseq_t& buffer, const HeaderRecord& record, uint16_t count, Logger* pLogger, IAPDUHandler* pHandler)
{
    // For Group 0 device attributes and other free-format objects,
    // each object is prefixed with a 2-byte length, followed by that many data bytes.
    // We deliver each object's data as an OctetString through the existing handler.

    switch (record.group)
    {
    case (0): {
        // Group 0: Device Attributes
        // Parse each free-format object, decode the attribute type/value, log it,
        // and deliver the raw data as OctetString through the existing handler.
        for (uint16_t i = 0; i < count; ++i)
        {
            if (buffer.length() < 2)
            {
                SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format object length");
                return ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS;
            }

            uint16_t dataLen = 0;
            ser4cpp::LittleEndian::read(buffer, dataLen);

            if (buffer.length() < dataLen)
            {
                SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format object data");
                return ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS;
            }

            const auto dataSlice = buffer.take(dataLen);

            // Parse and log the attribute type/value
            LogDeviceAttribute(dataSlice, record.variation, pLogger);

            // Deliver parsed attribute value through handler
            if (pHandler)
            {
                DeliverDeviceAttribute(dataSlice, 0, record.variation, pHandler);
            }

            buffer.advance(dataLen);
        }
        return ParseResult::OK;
    }
    case (70): {
        return ParseGroup70Objects(buffer, record, count, pLogger, pHandler);
    }
    default: {
        // Generic free-format handling for other groups
        // Skip the data for each object
        for (uint16_t i = 0; i < count; ++i)
        {
            if (buffer.length() < 2)
            {
                SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format object length");
                return ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS;
            }

            uint16_t dataLen = 0;
            ser4cpp::LittleEndian::read(buffer, dataLen);

            if (buffer.length() < dataLen)
            {
                SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format object data");
                return ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS;
            }

            buffer.advance(dataLen);
        }

        FORMAT_LOGGER_BLOCK(pLogger, flags::WARN, "Unsupported free-format object - %i / %i", record.group,
                            record.variation);
        return ParseResult::OK;
    }
    }
}

ParseResult FreeFormatParser::ParseGroup70Objects(
    ser4cpp::rseq_t& buffer, const HeaderRecord& record, uint16_t count, Logger* pLogger, IAPDUHandler* pHandler)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        if (buffer.length() < 2)
        {
            SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format object length");
            return ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS;
        }

        uint16_t dataLen = 0;
        ser4cpp::LittleEndian::read(buffer, dataLen);

        if (buffer.length() < dataLen)
        {
            SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for free-format object data");
            return ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS;
        }

        const auto dataSlice = buffer.take(dataLen);

        // Parse and log each variation's fields
        switch (record.variation)
        {
        case 2:
            LogGroup70Var2(dataSlice, pLogger);
            break;
        case 3:
            LogGroup70Var3(dataSlice, pLogger);
            break;
        case 4:
            LogGroup70Var4(dataSlice, pLogger);
            break;
        case 5:
            LogGroup70Var5(dataSlice, pLogger);
            break;
        case 6:
            LogGroup70Var6(dataSlice, pLogger);
            break;
        case 7:
            LogGroup70Var7(dataSlice, pLogger);
            break;
        case 8:
            LogGroup70Var8(dataSlice, pLogger);
            break;
        default:
            FORMAT_LOGGER_BLOCK(pLogger, flags::WARN, "Unknown Group70 variation: %u", record.variation);
            break;
        }

        // Deliver data through handler as OctetString
        if (pHandler)
        {
            DeliverOctetString(dataSlice, record, pHandler);
        }

        buffer.advance(dataLen);
    }

    return ParseResult::OK;
}

void FreeFormatParser::DeliverOctetString(ser4cpp::rseq_t data, const HeaderRecord& record, IAPDUHandler* pHandler)
{
    // Truncate to OctetData::MAX_SIZE if needed
    const auto len = static_cast<uint16_t>(std::min(static_cast<size_t>(data.length()), static_cast<size_t>(255)));
    OctetString octets(Buffer(data, len));

    auto read = [octets](ser4cpp::rseq_t& /*unused*/, uint32_t /*pos*/) -> Indexed<OctetString> {
        return WithIndex(octets, 0);
    };

    auto collection = CreateBufferedCollection<Indexed<OctetString>>(data, 1, read);
    auto range = Range::From(0, 0);
    pHandler->OnHeader(RangeHeader(record, range), collection);
}

void FreeFormatParser::LogGroup70Var2(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 2 - Authentication
    // Wire format: user_name_offset(u16), user_name_length(u16),
    //              password_offset(u16), password_length(u16),
    //              auth_key(u32), user_name(variable), password(variable)
    if (data.length() < 12)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Group70Var2: insufficient data for fixed fields");
        return;
    }

    uint16_t userNameOffset = 0;
    uint16_t userNameLength = 0;
    uint16_t passwordOffset = 0;
    uint16_t passwordLength = 0;
    uint32_t authKey = 0;

    ser4cpp::LittleEndian::read(data, userNameOffset);
    ser4cpp::LittleEndian::read(data, userNameLength);
    ser4cpp::LittleEndian::read(data, passwordOffset);
    ser4cpp::LittleEndian::read(data, passwordLength);
    ser4cpp::LittleEndian::read(data, authKey);

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                        "Group70Var2 Authentication - auth_key: %u, user_name_len: %u, password_len: %u", authKey,
                        userNameLength, passwordLength);
}

void FreeFormatParser::LogGroup70Var3(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 3 - File Command
    // Wire format: file_name_offset(u16), file_name_length(u16),
    //              time_of_creation(6 bytes), permissions(u16),
    //              auth_key(u32), file_size(u32), mode(u16),
    //              max_block_size(u16), request_id(u16), file_name(variable)
    if (data.length() < 26)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Group70Var3: insufficient data for fixed fields");
        return;
    }

    uint16_t fileNameOffset = 0;
    uint16_t fileNameLength = 0;
    ser4cpp::LittleEndian::read(data, fileNameOffset);
    ser4cpp::LittleEndian::read(data, fileNameLength);

    // time_of_creation: 6 bytes (48-bit DNP3 timestamp)
    uint64_t timeOfCreation = 0;
    for (int j = 0; j < 6; ++j)
    {
        timeOfCreation |= static_cast<uint64_t>(data[0]) << (j * 8);
        data.advance(1);
    }

    uint16_t permissions = 0;
    uint32_t authKey = 0;
    uint32_t fileSize = 0;
    uint16_t mode = 0;
    uint16_t maxBlockSize = 0;
    uint16_t requestId = 0;

    ser4cpp::LittleEndian::read(data, permissions);
    ser4cpp::LittleEndian::read(data, authKey);
    ser4cpp::LittleEndian::read(data, fileSize);
    ser4cpp::LittleEndian::read(data, mode);
    ser4cpp::LittleEndian::read(data, maxBlockSize);
    ser4cpp::LittleEndian::read(data, requestId);

    // Extract filename if available
    char fileName[256] = {0};
    const auto nameLen
        = std::min(static_cast<size_t>(fileNameLength), std::min(data.length(), static_cast<size_t>(255)));
    if (nameLen > 0)
    {
        std::memcpy(fileName, data, nameLen);
    }

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                        "Group70Var3 FileCommand - file: \"%s\", size: %u, mode: %u, "
                        "max_block: %u, req_id: %u, auth_key: %u",
                        fileName, fileSize, mode, maxBlockSize, requestId, authKey);
}

void FreeFormatParser::LogGroup70Var4(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 4 - File Command Status
    // Wire format: file_handle(u32), file_size(u32), max_block_size(u16),
    //              request_id(u16), status_code(u8), optional_text(variable)
    if (data.length() < 13)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Group70Var4: insufficient data for fixed fields");
        return;
    }

    uint32_t fileHandle = 0;
    uint32_t fileSize = 0;
    uint16_t maxBlockSize = 0;
    uint16_t requestId = 0;

    ser4cpp::LittleEndian::read(data, fileHandle);
    ser4cpp::LittleEndian::read(data, fileSize);
    ser4cpp::LittleEndian::read(data, maxBlockSize);
    ser4cpp::LittleEndian::read(data, requestId);

    uint8_t statusCode = data[0];
    data.advance(1);

    char text[256] = {0};
    const auto textLen = std::min(data.length(), static_cast<size_t>(255));
    if (textLen > 0)
    {
        std::memcpy(text, data, textLen);
    }

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                        "Group70Var4 FileCommandStatus - handle: %u, size: %u, max_block: %u, "
                        "req_id: %u, status: %u, text: \"%s\"",
                        fileHandle, fileSize, maxBlockSize, requestId, statusCode, text);
}

void FreeFormatParser::LogGroup70Var5(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 5 - File Transport
    // Wire format: file_handle(u32), block_number(u32), file_data(variable)
    if (data.length() < 8)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Group70Var5: insufficient data for fixed fields");
        return;
    }

    uint32_t fileHandle = 0;
    uint32_t blockNumber = 0;

    ser4cpp::LittleEndian::read(data, fileHandle);
    ser4cpp::LittleEndian::read(data, blockNumber);

    // The last bit of block_number indicates final block
    const bool isFinal = (blockNumber & 0x80000000) != 0;
    const uint32_t blockNum = blockNumber & 0x7FFFFFFF;

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                        "Group70Var5 FileTransport - handle: %u, block: %u, final: %s, data_len: %zu", fileHandle,
                        blockNum, isFinal ? "true" : "false", data.length());
}

void FreeFormatParser::LogGroup70Var6(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 6 - File Transport Status
    // Wire format: file_handle(u32), block_number(u32), status_code(u8), optional_text(variable)
    if (data.length() < 9)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Group70Var6: insufficient data for fixed fields");
        return;
    }

    uint32_t fileHandle = 0;
    uint32_t blockNumber = 0;

    ser4cpp::LittleEndian::read(data, fileHandle);
    ser4cpp::LittleEndian::read(data, blockNumber);

    uint8_t statusCode = data[0];
    data.advance(1);

    char text[256] = {0};
    const auto textLen = std::min(data.length(), static_cast<size_t>(255));
    if (textLen > 0)
    {
        std::memcpy(text, data, textLen);
    }

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                        "Group70Var6 FileTransportStatus - handle: %u, block: %u, status: %u, text: \"%s\"", fileHandle,
                        blockNumber, statusCode, text);
}

void FreeFormatParser::LogGroup70Var7(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 7 - File Descriptor
    // Wire format: file_name_offset(u16), file_name_length(u16),
    //              file_type(u16), file_size(u32),
    //              time_of_creation(6 bytes), permissions(u16),
    //              request_id(u16), file_name(variable)
    if (data.length() < 20)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Group70Var7: insufficient data for fixed fields");
        return;
    }

    uint16_t fileNameOffset = 0;
    uint16_t fileNameLength = 0;
    uint16_t fileType = 0;
    uint32_t fileSize = 0;

    ser4cpp::LittleEndian::read(data, fileNameOffset);
    ser4cpp::LittleEndian::read(data, fileNameLength);
    ser4cpp::LittleEndian::read(data, fileType);
    ser4cpp::LittleEndian::read(data, fileSize);

    // time_of_creation: 6 bytes (48-bit DNP3 timestamp)
    uint64_t timeOfCreation = 0;
    for (int j = 0; j < 6; ++j)
    {
        timeOfCreation |= static_cast<uint64_t>(data[0]) << (j * 8);
        data.advance(1);
    }

    uint16_t permissions = 0;
    uint16_t requestId = 0;

    ser4cpp::LittleEndian::read(data, permissions);
    ser4cpp::LittleEndian::read(data, requestId);

    char fileName[256] = {0};
    const auto nameLen
        = std::min(static_cast<size_t>(fileNameLength), std::min(data.length(), static_cast<size_t>(255)));
    if (nameLen > 0)
    {
        std::memcpy(fileName, data, nameLen);
    }

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                        "Group70Var7 FileDescriptor - file: \"%s\", type: %u, size: %u, req_id: %u", fileName, fileType,
                        fileSize, requestId);
}

void FreeFormatParser::LogGroup70Var8(ser4cpp::rseq_t data, Logger* pLogger)
{
    // Group 70 Var 8 - File Specification String
    // Wire format: file_specification(variable, entire data is the string)
    char fileSpec[256] = {0};
    const auto specLen = std::min(data.length(), static_cast<size_t>(255));
    if (specLen > 0)
    {
        std::memcpy(fileSpec, data, specLen);
    }

    FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group70Var8 FileSpecification - \"%s\"", fileSpec);
}

const char* FreeFormatParser::GetAttrDataTypeName(uint8_t typeCode)
{
    switch (typeCode)
    {
    case 1:
        return "VisibleString";
    case 2:
        return "UnsignedInt";
    case 3:
        return "SignedInt";
    case 4:
        return "FloatingPoint";
    case 5:
        return "OctetString";
    case 6:
        return "BitString";
    case 7:
        return "DNP3Time";
    case 254:
        return "AttrList";
    case 255:
        return "ExtAttrList";
    default:
        return "Unknown";
    }
}

void FreeFormatParser::LogDeviceAttribute(ser4cpp::rseq_t data, uint8_t variation, Logger* pLogger)
{
    // Device attribute wire format (IEEE 1815-2012):
    // type_code(u8) + length(u8) + payload(length bytes)
    if (data.length() < 2)
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - raw data (%zu bytes)",
                            variation, data.length());
        return;
    }

    const uint8_t typeCode = data[0];
    const uint8_t payloadLen = data[1];
    const char* typeName = GetAttrDataTypeName(typeCode);

    if (data.length() < static_cast<size_t>(2 + payloadLen))
    {
        FORMAT_LOGGER_BLOCK(
            pLogger, flags::WARN,
            "Group0Var%u DeviceAttribute - type: %s (%u), declared length %u but only %zu bytes available", variation,
            typeName, typeCode, payloadLen, data.length() - 2);
        return;
    }

    // Advance past the type code and length
    data.advance(2);

    switch (typeCode)
    {
    case 1: // VisibleString
    {
        char strBuf[256] = {0};
        const auto len = std::min(static_cast<size_t>(payloadLen), static_cast<size_t>(255));
        if (len > 0)
        {
            std::memcpy(strBuf, data, len);
        }
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                            "Group0Var%u DeviceAttribute - type: VisibleString, value: \"%s\"", variation, strBuf);
        break;
    }
    case 2: // UnsignedInt
    {
        uint32_t value = 0;
        if (payloadLen == 1 && data.length() >= 1)
        {
            value = data[0];
        }
        else if (payloadLen == 2 && data.length() >= 2)
        {
            uint16_t v16 = 0;
            ser4cpp::LittleEndian::read(data, v16);
            value = v16;
        }
        else if (payloadLen == 4 && data.length() >= 4)
        {
            ser4cpp::LittleEndian::read(data, value);
        }
        else
        {
            FORMAT_LOGGER_BLOCK(pLogger, flags::WARN,
                                "Group0Var%u DeviceAttribute - type: UnsignedInt, unsupported length: %u", variation,
                                payloadLen);
            break;
        }
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - type: UnsignedInt, value: %u",
                            variation, value);
        break;
    }
    case 3: // SignedInt
    {
        int32_t value = 0;
        if (payloadLen == 1 && data.length() >= 1)
        {
            value = static_cast<int8_t>(data[0]);
        }
        else if (payloadLen == 2 && data.length() >= 2)
        {
            uint16_t raw = 0;
            ser4cpp::LittleEndian::read(data, raw);
            value = static_cast<int16_t>(raw);
        }
        else if (payloadLen == 4 && data.length() >= 4)
        {
            uint32_t raw = 0;
            ser4cpp::LittleEndian::read(data, raw);
            value = static_cast<int32_t>(raw);
        }
        else
        {
            FORMAT_LOGGER_BLOCK(pLogger, flags::WARN,
                                "Group0Var%u DeviceAttribute - type: SignedInt, unsupported length: %u", variation,
                                payloadLen);
            break;
        }
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - type: SignedInt, value: %d",
                            variation, value);
        break;
    }
    case 4: // FloatingPoint
    {
        if (payloadLen == 4 && data.length() >= 4)
        {
            float f32;
            std::memcpy(&f32, data, 4);
            FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - type: Float32, value: %f",
                                variation, static_cast<double>(f32));
        }
        else if (payloadLen == 8 && data.length() >= 8)
        {
            double f64;
            std::memcpy(&f64, data, 8);
            FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - type: Float64, value: %f",
                                variation, f64);
        }
        else
        {
            FORMAT_LOGGER_BLOCK(pLogger, flags::WARN,
                                "Group0Var%u DeviceAttribute - type: FloatingPoint, unsupported length: %u", variation,
                                payloadLen);
        }
        break;
    }
    case 5: // OctetString
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                            "Group0Var%u DeviceAttribute - type: OctetString, length: %u", variation, payloadLen);
        break;
    }
    case 6: // BitString
    {
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - type: BitString, length: %u",
                            variation, payloadLen);
        break;
    }
    case 7: // DNP3Time
    {
        if (payloadLen == 6 && data.length() >= 6)
        {
            uint64_t timestamp = 0;
            for (int j = 0; j < 6; ++j)
            {
                timestamp |= static_cast<uint64_t>(data[j]) << (j * 8);
            }
            FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                                "Group0Var%u DeviceAttribute - type: DNP3Time, value: %llu ms", variation,
                                static_cast<unsigned long long>(timestamp));
        }
        else
        {
            FORMAT_LOGGER_BLOCK(pLogger, flags::WARN,
                                "Group0Var%u DeviceAttribute - type: DNP3Time, invalid length: %u (expected 6)",
                                variation, payloadLen);
        }
        break;
    }
    case 254: // AttrList
    case 255: // ExtAttrList
    {
        // AttrList is pairs of (variation_u8, properties_u8)
        // ExtAttrList has actual length = payloadLen + 256
        const uint16_t actualLen = (typeCode == 255) ? static_cast<uint16_t>(payloadLen) + 256 : payloadLen;
        const uint16_t numPairs = actualLen / 2;
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX, "Group0Var%u DeviceAttribute - type: %s, entries: %u",
                            variation, typeName, numPairs);
        break;
    }
    default: {
        FORMAT_LOGGER_BLOCK(pLogger, flags::APP_OBJECT_RX,
                            "Group0Var%u DeviceAttribute - unknown type code: %u, length: %u", variation, typeCode,
                            payloadLen);
        break;
    }
    }
}

void FreeFormatParser::DeliverDeviceAttribute(ser4cpp::rseq_t data,
                                              uint8_t set,
                                              uint8_t variation,
                                              IAPDUHandler* pHandler)
{
    DeviceAttributeValue attr;

    if (data.length() < 2)
    {
        // Too short to decode, deliver raw
        attr.type = DeviceAttrType::OCTET_STRING;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + data.length());
        pHandler->OnDeviceAttribute(set, variation, attr);
        return;
    }

    const uint8_t typeCode = data[0];
    const uint8_t payloadLen = data[1];
    data.advance(2);

    if (data.length() < payloadLen)
    {
        // Truncated payload, deliver raw
        attr.type = DeviceAttrType::OCTET_STRING;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + data.length());
        pHandler->OnDeviceAttribute(set, variation, attr);
        return;
    }

    switch (typeCode)
    {
    case 1: // VisibleString
    {
        attr.type = DeviceAttrType::VISIBLE_STRING;
        attr.stringValue.assign(reinterpret_cast<const char*>(static_cast<const uint8_t*>(data)), payloadLen);
        break;
    }
    case 2: // UnsignedInt
    {
        attr.type = DeviceAttrType::UNSIGNED_INT;
        attr.unsignedValue = 0;
        if (payloadLen == 1 && data.length() >= 1)
        {
            attr.unsignedValue = data[0];
        }
        else if (payloadLen == 2 && data.length() >= 2)
        {
            uint16_t v16 = 0;
            auto tmp = data;
            ser4cpp::LittleEndian::read(tmp, v16);
            attr.unsignedValue = v16;
        }
        else if (payloadLen == 4 && data.length() >= 4)
        {
            uint32_t v32 = 0;
            auto tmp = data;
            ser4cpp::LittleEndian::read(tmp, v32);
            attr.unsignedValue = v32;
        }
        break;
    }
    case 3: // SignedInt
    {
        attr.type = DeviceAttrType::SIGNED_INT;
        attr.signedValue = 0;
        if (payloadLen == 1 && data.length() >= 1)
        {
            attr.signedValue = static_cast<int8_t>(data[0]);
        }
        else if (payloadLen == 2 && data.length() >= 2)
        {
            uint16_t raw = 0;
            auto tmp = data;
            ser4cpp::LittleEndian::read(tmp, raw);
            attr.signedValue = static_cast<int16_t>(raw);
        }
        else if (payloadLen == 4 && data.length() >= 4)
        {
            uint32_t raw = 0;
            auto tmp = data;
            ser4cpp::LittleEndian::read(tmp, raw);
            attr.signedValue = static_cast<int32_t>(raw);
        }
        break;
    }
    case 4: // FloatingPoint
    {
        attr.type = DeviceAttrType::FLOATING_POINT;
        if (payloadLen == 4 && data.length() >= 4)
        {
            float f32;
            std::memcpy(&f32, data, 4);
            attr.floatValue = static_cast<double>(f32);
        }
        else if (payloadLen == 8 && data.length() >= 8)
        {
            double f64;
            std::memcpy(&f64, data, 8);
            attr.floatValue = f64;
        }
        break;
    }
    case 5: // OctetString
    {
        attr.type = DeviceAttrType::OCTET_STRING;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + payloadLen);
        break;
    }
    case 6: // BitString
    {
        attr.type = DeviceAttrType::BIT_STRING;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + payloadLen);
        break;
    }
    case 7: // DNP3Time
    {
        attr.type = DeviceAttrType::DNP3_TIME;
        if (payloadLen == 6 && data.length() >= 6)
        {
            attr.timeValue = 0;
            for (int j = 0; j < 6; ++j)
            {
                attr.timeValue |= static_cast<uint64_t>(data[j]) << (j * 8);
            }
        }
        break;
    }
    case 254: // AttrList
    {
        attr.type = DeviceAttrType::ATTR_LIST;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + payloadLen);
        break;
    }
    case 255: // ExtAttrList
    {
        attr.type = DeviceAttrType::EXT_ATTR_LIST;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + payloadLen);
        break;
    }
    default: {
        attr.type = DeviceAttrType::OCTET_STRING;
        attr.rawValue.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + payloadLen);
        break;
    }
    }

    pHandler->OnDeviceAttribute(set, variation, attr);
}

} // namespace opendnp3
