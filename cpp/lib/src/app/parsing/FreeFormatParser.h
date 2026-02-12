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
#ifndef OPENDNP3_FREEFORMATPARSER_H
#define OPENDNP3_FREEFORMATPARSER_H

#include "app/GroupVariationRecord.h"
#include "app/parsing/IAPDUHandler.h"
#include "app/parsing/ParseResult.h"
#include "app/parsing/ParserSettings.h"

#include "opendnp3/logging/Logger.h"

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

class FreeFormatParser
{
public:
    static ParseResult ParseHeader(ser4cpp::rseq_t& buffer,
                                   const ParserSettings& settings,
                                   const HeaderRecord& record,
                                   Logger* pLogger,
                                   IAPDUHandler* pHandler);

private:
    // Handler function type for Group120 free-format objects
    typedef bool (&FreeFormatHandler)(const FreeFormatHeader& header,
                                      const ser4cpp::rseq_t& objects,
                                      IAPDUHandler* pHandler);

    // Template that parses a single Group120 variable-length object and delivers it to the handler
    template<class T>
    static bool ParseAny(const FreeFormatHeader& header, const ser4cpp::rseq_t& object, IAPDUHandler* pHandler)
    {
        T value;
        auto success = value.Read(object);
        if (success && pHandler)
        {
            pHandler->OnHeader(header, value, object);
        }
        return success;
    }

    static ParseResult ParseFreeFormat(FreeFormatHandler parser,
                                       const FreeFormatHeader& header,
                                       uint16_t size,
                                       ser4cpp::rseq_t& objects,
                                       IAPDUHandler* pHandler,
                                       Logger* pLogger);

    static ParseResult ParseGroup120(ser4cpp::rseq_t& buffer,
                                     const ParserSettings& settings,
                                     const HeaderRecord& record,
                                     Logger* pLogger,
                                     IAPDUHandler* pHandler);

    static ParseResult ParseFreeFormatObjects(
        ser4cpp::rseq_t& buffer, const HeaderRecord& record, uint16_t count, Logger* pLogger, IAPDUHandler* pHandler);

    static ParseResult ParseGroup70Objects(
        ser4cpp::rseq_t& buffer, const HeaderRecord& record, uint16_t count, Logger* pLogger, IAPDUHandler* pHandler);

    static ParseResult ParseDataSetObjects(
        ser4cpp::rseq_t& buffer, const HeaderRecord& record, uint16_t count, Logger* pLogger, IAPDUHandler* pHandler);

    static void LogGroup70Var2(ser4cpp::rseq_t data, Logger* pLogger);
    static void LogGroup70Var3(ser4cpp::rseq_t data, Logger* pLogger);
    static void LogGroup70Var4(ser4cpp::rseq_t data, Logger* pLogger);
    static void LogGroup70Var5(ser4cpp::rseq_t data, Logger* pLogger);
    static void LogGroup70Var6(ser4cpp::rseq_t data, Logger* pLogger);
    static void LogGroup70Var7(ser4cpp::rseq_t data, Logger* pLogger);
    static void LogGroup70Var8(ser4cpp::rseq_t data, Logger* pLogger);

    static void DeliverOctetString(ser4cpp::rseq_t data, const HeaderRecord& record, IAPDUHandler* pHandler);

    static void LogDeviceAttribute(ser4cpp::rseq_t data, uint8_t variation, Logger* pLogger);

    static void DeliverDeviceAttribute(ser4cpp::rseq_t data, uint8_t set, uint8_t variation, IAPDUHandler* pHandler);

    static const char* GetAttrDataTypeName(uint8_t typeCode);
};

} // namespace opendnp3

#endif
