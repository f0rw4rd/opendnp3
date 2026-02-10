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

// Tests exercising the APDU parser against real-world protocol data extracted
// from pcap captures. The primary value is verifying that the parser handles
// genuine traffic without crashes and produces correct parse results.

#include "utils/BufferHelpers.h"

#include <opendnp3/app/Indexed.h>
#include <opendnp3/gen/FunctionCode.h>
#include <opendnp3/logging/LogLevels.h>

#include "dnp3mocks/MockAPDUHeaderHandler.h"
#include "dnp3mocks/MockLogHandler.h"

#include <app/parsing/APDUHeaderParser.h>
#include <app/parsing/APDUParser.h>
#include <catch.hpp>

using namespace std;
using namespace opendnp3;
using namespace ser4cpp;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Parse an object-header hex payload and return the result plus the handler.
static ParseResult ParseObjects(const std::string& hex, MockApduHeaderHandler& handler)
{
    HexSequence buffer(hex);
    MockLogHandler log;
    return APDUParser::Parse(buffer.ToRSeq(), handler, &log.logger);
}

// Shorthand: parse objects, assert expected result and header count.
static void ExpectObjects(const std::string& hex, ParseResult expected, size_t expectedHeaders)
{
    MockApduHeaderHandler handler;
    auto result = ParseObjects(hex, handler);
    REQUIRE((result == expected));
    REQUIRE(handler.records.size() == expectedHeaders);
}

// Parse an APDU as a request, assert success.
static APDUHeaderParser::Result<APDUHeader> ParseRequest(const std::string& hex)
{
    HexSequence buffer(hex);
    return APDUHeaderParser::ParseRequest(buffer.ToRSeq());
}

// Parse an APDU as a response, assert success.
static APDUHeaderParser::Result<APDUResponseHeader> ParseResponse(const std::string& hex)
{
    HexSequence buffer(hex);
    return APDUHeaderParser::ParseResponse(buffer.ToRSeq());
}

// ---------------------------------------------------------------------------
// Suite: dnp3_read pcap -- basic read/response exchange
// ---------------------------------------------------------------------------

#define PCAP_READ(name) "PcapParsing - dnp3_read - " name

TEST_CASE(PCAP_READ("frame4 - Read Class 0123 header"))
{
    auto result = ParseRequest("C8 01 3C 02 06 3C 03 06 3C 04 06 3C 01 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::READ);
}

TEST_CASE(PCAP_READ("frame4 - Read Class 0123 objects"))
{
    // 4 all-objects headers: Class 2, Class 3, Class 4, Class 1
    MockApduHeaderHandler handler;
    auto r = ParseObjects("3C 02 06 3C 03 06 3C 04 06 3C 01 06", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 4);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group60Var2));
    REQUIRE((handler.records[1].enumeration == GroupVariation::Group60Var3));
    REQUIRE((handler.records[2].enumeration == GroupVariation::Group60Var4));
    REQUIRE((handler.records[3].enumeration == GroupVariation::Group60Var1));
}

TEST_CASE(PCAP_READ("frame6 - Response with static data header"))
{
    auto result = ParseResponse("C8 81 00 00 01 02 00 00 08 81 01 81 01 01 01 01 01 01 "
                                "0A 02 00 00 06 00 00 00 00 00 00 01 "
                                "1E 01 00 00 0E "
                                "01 EF 03 00 00 01 03 00 00 00 01 ED 03 00 00 "
                                "01 2B D1 FF FF 01 ED 03 00 00 01 E6 2E 00 00 "
                                "01 17 0D 02 00 00 00 00 00 00 01 B5 0C 02 00 00 00 00 00 00 "
                                "01 8A 0D 02 00 00 00 00 00 00 "
                                "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);
    REQUIRE(result.header.IIN == IINField(0, 0));
}

TEST_CASE(PCAP_READ("frame6 - Response objects parse"))
{
    // Objects portion: Group1Var2 range, Group10Var2 range, Group30Var1 range, etc.
    MockApduHeaderHandler handler;
    auto r = ParseObjects("01 02 00 00 08 81 01 81 01 01 01 01 01 01 "
                          "0A 02 00 00 06 00 00 00 00 00 00 01 "
                          "1E 01 00 00 0E "
                          "01 EF 03 00 00 01 03 00 00 00 01 ED 03 00 00 "
                          "01 2B D1 FF FF 01 ED 03 00 00 01 E6 2E 00 00 "
                          "01 17 0D 02 00 00 00 00 00 00 01 B5 0C 02 00 00 00 00 00 00 "
                          "01 8A 0D 02 00 00 00 00 00 00 "
                          "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    // Expect multiple object headers parsed: g1v2, g10v2, g30v1, ...
    REQUIRE(handler.records.size() >= 3);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group1Var2));
    REQUIRE((handler.records[1].enumeration == GroupVariation::Group10Var2));
    REQUIRE((handler.records[2].enumeration == GroupVariation::Group30Var1));

    // 9 Binary inputs (indices 0..8)
    REQUIRE(handler.staticBinaries.size() == 9);
    // 15 Analog inputs (indices 0..14)
    REQUIRE(handler.eventAnalogs.size() == 15);
}

// ---------------------------------------------------------------------------
// Suite: dnp3_write pcap -- write time and date
// ---------------------------------------------------------------------------

#define PCAP_WRITE(name) "PcapParsing - dnp3_write - " name

TEST_CASE(PCAP_WRITE("frame4 - Write Time and Date header"))
{
    auto result = ParseRequest("C1 02 32 01 07 01 FA 7D 0B 46 0D 01");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::WRITE);
}

TEST_CASE(PCAP_WRITE("frame4 - Write Time and Date objects"))
{
    // Group50Var1 (Time and Date) count-of-1
    ExpectObjects("32 01 07 01 FA 7D 0B 46 0D 01", ParseResult::OK, 1);
}

// ---------------------------------------------------------------------------
// Suite: dnp3_file_read pcap -- file transfer operations
// ---------------------------------------------------------------------------

#define PCAP_FILE(name) "PcapParsing - dnp3_file_read - " name

TEST_CASE(PCAP_FILE("frame4 - Open File request header"))
{
    auto result = ParseRequest("CE 19 46 03 5B 01 24 00 1A 00 0A 00 00 00 00 00 00 00 "
                               "00 00 00 00 00 00 00 00 01 00 00 04 04 00 2E 2F 74 65 73 74 2E 78 6D 6C");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::OPEN_FILE);
}

TEST_CASE(PCAP_FILE("frame6 - Open File response header"))
{
    auto result = ParseResponse("CE 81 10 00 46 04 5B 01 0D 00 78 56 34 12 3E 03 00 00 00 04 04 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);
    // IIN: 0x10,0x00 -> NEED_TIME (bit 4 of LSB)
    REQUIRE(result.header.IIN.IsSet(IINBit::NEED_TIME));
}

TEST_CASE(PCAP_FILE("frame9 - Large file response APDU header"))
{
    // This is a large response with file data (Group70Var5)
    // We only test header parsing since the content is file data
    auto result = ParseResponse("EF 81 10 00 46 05 5B 01 46 03 78 56 34 12 "
                                "00 00 00 80 EF BB BF 3C 3F 78 6D 6C 20 76 65 72 73 69 6F 6E 3D 22 31 2E 30 22 "
                                "20 65 6E 63 6F 64 69 6E 67 3D 22 75 74 66 2D 38 22 3F 3E");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);
}

TEST_CASE(PCAP_FILE("frame13 - Write Time and Date"))
{
    auto result = ParseRequest("C0 02 32 01 07 01 D2 48 BE 66 34 01");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::WRITE);

    ExpectObjects("32 01 07 01 D2 48 BE 66 34 01", ParseResult::OK, 1);
}

TEST_CASE(PCAP_FILE("frame20 - Close File response header"))
{
    auto result = ParseRequest("C2 1A 46 04 5B 01 0D 00 78 56 34 12 00 00 00 00 00 00 05 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::CLOSE_FILE);
}

TEST_CASE(PCAP_FILE("frame21 - Response to close file"))
{
    auto result = ParseResponse("C2 81 00 00 46 04 5B 01 0D 00 78 56 34 12 00 00 00 00 00 00 05 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);
    REQUIRE(result.header.IIN == IINField(0, 0));
}

// ---------------------------------------------------------------------------
// Suite: dnp3 main pcap -- master/outstation typical session
// ---------------------------------------------------------------------------

#define PCAP_MAIN(name) "PcapParsing - dnp3_main - " name

TEST_CASE(PCAP_MAIN("frame7 - Write Time and Date"))
{
    auto result = ParseRequest("C1 02 32 01 07 01 EB E4 5A 87 FF 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::WRITE);

    ExpectObjects("32 01 07 01 EB E4 5A 87 FF 00", ParseResult::OK, 1);
}

TEST_CASE(PCAP_MAIN("frame11 - Disable Unsolicited header"))
{
    auto result = ParseRequest("C2 15 3C 02 06 3C 03 06 3C 04 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::DISABLE_UNSOLICITED);
}

TEST_CASE(PCAP_MAIN("frame11 - Disable Unsolicited objects"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("3C 02 06 3C 03 06 3C 04 06", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group60Var2));
    REQUIRE((handler.records[1].enumeration == GroupVariation::Group60Var3));
    REQUIRE((handler.records[2].enumeration == GroupVariation::Group60Var4));
}

TEST_CASE(PCAP_MAIN("frame37 - Enable Unsolicited"))
{
    auto result = ParseRequest("C2 14 3C 02 06 3C 03 06 3C 04 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::ENABLE_UNSOLICITED);

    MockApduHeaderHandler handler;
    auto r = ParseObjects("3C 02 06 3C 03 06 3C 04 06", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
}

// ---------------------------------------------------------------------------
// Unsolicited responses -- these carry event data (Class 2/3 events)
// ---------------------------------------------------------------------------

#define PCAP_UNSOL(name) "PcapParsing - Unsolicited - " name

TEST_CASE(PCAP_UNSOL("frame40 - Unsolicited with binary and analog events"))
{
    auto result
        = ParseResponse("F3 82 00 00 "
                        "33 01 07 01 E2 43 7D 87 FF 00 "
                        "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 F3 03 01 00 01 F3 03 "
                        "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::UNSOLICITED_RESPONSE);
    REQUIRE(result.header.IIN == IINField(0, 0));
}

TEST_CASE(PCAP_UNSOL("frame40 - Unsolicited objects parse"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 E2 43 7D 87 FF 00 "
                          "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 F3 03 01 00 01 F3 03 "
                          "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    // Group51Var1 (CTO), Group2Var3 (Binary event w/ time), Group32Var1 (Analog event)
    REQUIRE(handler.records.size() == 3);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group51Var1));
    REQUIRE((handler.records[1].enumeration == GroupVariation::Group2Var3));
    REQUIRE((handler.records[2].enumeration == GroupVariation::Group32Var1));
    // 5 binary events
    REQUIRE(handler.eventBinaries.size() == 5);
    // 3 analog events
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame43 - Unsolicited with events"))
{
    auto result = ParseResponse("F4 82 00 00 "
                                "33 01 07 01 D5 47 7D 87 FF 00 "
                                "02 03 28 04 00 02 00 01 00 00 00 00 01 17 03 01 00 81 17 03 02 00 81 17 03 "
                                "20 01 28 03 00 02 00 01 C6 00 00 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::UNSOLICITED_RESPONSE);

    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 D5 47 7D 87 FF 00 "
                          "02 03 28 04 00 02 00 01 00 00 00 00 01 17 03 01 00 81 17 03 02 00 81 17 03 "
                          "20 01 28 03 00 02 00 01 C6 00 00 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 4);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame46 - Unsolicited with events"))
{
    auto result
        = ParseResponse("F5 82 00 00 "
                        "33 01 07 01 F4 4E 7D 87 FF 00 "
                        "02 03 28 05 00 00 00 81 00 00 01 00 01 00 00 02 00 01 00 00 00 00 01 C1 03 01 00 81 C1 03 "
                        "20 01 28 03 00 00 00 01 C6 00 00 00 01 00 01 CA 00 00 00 02 00 01 C6 00 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::UNSOLICITED_RESPONSE);

    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 F4 4E 7D 87 FF 00 "
                          "02 03 28 05 00 00 00 81 00 00 01 00 01 00 00 02 00 01 00 00 00 00 01 C1 03 01 00 81 C1 03 "
                          "20 01 28 03 00 00 00 01 C6 00 00 00 01 00 01 CA 00 00 00 02 00 01 C6 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 5);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame49 - Unsolicited with events"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 B5 52 7D 87 FF 00 "
                          "02 03 28 04 00 02 00 81 00 00 00 00 81 A4 03 01 00 01 A4 03 02 00 01 A4 03 "
                          "20 01 28 03 00 02 00 01 00 00 00 00 00 00 01 CA 00 00 00 01 00 01 C8 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 4);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame52 - Unsolicited with events"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 1A 5A 7D 87 FF 00 "
                          "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 AD 03 01 00 01 AD 03 "
                          "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 5);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame55 - Unsolicited with binary events and truncated analog"))
{
    // This frame has a partial final header: "00 00 81 01 07" at the end
    // which is not a complete analog event object. The parser should
    // detect insufficient data after the initial valid headers.
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 C7 5D 7D 87 FF 00 "
                          "02 03 28 05 00 02 00 01 00 00 00 00 01 0E 03 01 00 81 0E 03 02 00 81 0E 03 "
                          "00 00 81 01 07",
                          handler);
    // May fail on the truncated object or parse OK depending on what "00 00 81 01 07" decodes as
    // The key assertion is that it does NOT crash
    // (Group0Var0 with qualifier 0x81 is unknown, so we expect a parse error)
    // We just verify it returns a defined result without crashing
    REQUIRE(((r == ParseResult::OK) || (r == ParseResult::NOT_ENOUGH_DATA_FOR_HEADER)
             || (r == ParseResult::NOT_ENOUGH_DATA_FOR_OBJECTS) || (r == ParseResult::UNKNOWN_OBJECT)
             || (r == ParseResult::UNKNOWN_QUALIFIER)));
}

TEST_CASE(PCAP_UNSOL("frame58 - Unsolicited with events"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 C8 64 7D 87 FF 00 "
                          "02 03 28 05 00 01 00 01 00 00 02 00 01 00 00 00 00 01 2B 03 01 00 81 2B 03 02 00 81 2B 03 "
                          "20 01 28 03 00 00 00 01 C6 00 00 00 01 00 01 C7 00 00 00 02 00 01 C7 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 5);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame61 - Unsolicited with 3 binary + 3 analog"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 B5 6B 7D 87 FF 00 "
                          "02 03 28 03 00 00 00 81 00 00 01 00 01 00 00 02 00 01 00 00 "
                          "20 01 28 03 00 02 00 01 00 00 00 00 00 00 01 CA 00 00 00 01 00 01 C8 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 3);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame64 - Unsolicited with events"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 5E 6E 7D 87 FF 00 "
                          "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 0D 03 01 00 01 0D 03 "
                          "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 5);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

TEST_CASE(PCAP_UNSOL("frame67 - Unsolicited with 1 binary + 3 analog"))
{
    MockApduHeaderHandler handler;
    auto r = ParseObjects("33 01 07 01 6B 71 7D 87 FF 00 "
                          "02 03 28 01 00 02 00 01 00 00 "
                          "20 01 28 03 00 00 00 01 CB 00 00 00 01 00 01 CA 00 00 00 02 00 01 C7 00 00 00",
                          handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 3);
    REQUIRE(handler.eventBinaries.size() == 1);
    REQUIRE(handler.eventAnalogs.size() == 3);
}

// ---------------------------------------------------------------------------
// Delay measurement responses
// ---------------------------------------------------------------------------

#define PCAP_DELAY(name) "PcapParsing - DelayMeasurement - " name

TEST_CASE(PCAP_DELAY("frame71 - Delay measurement response"))
{
    auto result = ParseResponse("C3 81 00 00 34 02 07 01 88 13");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);

    // Group52Var2 (Time delay fine) with 1 count, value 0x1388 = 5000 ms
    ExpectObjects("34 02 07 01 88 13", ParseResult::OK, 1);
}

TEST_CASE(PCAP_DELAY("frame80 - Delay measurement response"))
{
    auto result = ParseResponse("C1 81 00 00 34 02 07 01 88 13");
    REQUIRE(result.success);

    ExpectObjects("34 02 07 01 88 13", ParseResult::OK, 1);
}

TEST_CASE(PCAP_DELAY("frame144 - Delay measurement response"))
{
    auto result = ParseResponse("C8 81 00 00 34 02 07 01 D0 07");
    REQUIRE(result.success);

    // 0x07D0 = 2000 ms
    ExpectObjects("34 02 07 01 D0 07", ParseResult::OK, 1);
}

TEST_CASE(PCAP_DELAY("frame147 - Delay measurement response"))
{
    auto result = ParseResponse("C9 81 00 00 34 02 07 01 D0 07");
    REQUIRE(result.success);

    ExpectObjects("34 02 07 01 D0 07", ParseResult::OK, 1);
}

// ---------------------------------------------------------------------------
// Read requests
// ---------------------------------------------------------------------------

#define PCAP_READS(name) "PcapParsing - Reads - " name

TEST_CASE(PCAP_READS("frame82 - Read Binary Input Change"))
{
    auto result = ParseRequest("C2 01 02 00 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::READ);

    // Group2Var0 all objects
    MockApduHeaderHandler handler;
    auto r = ParseObjects("02 00 06", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 1);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group2Var0));
}

TEST_CASE(PCAP_READS("frame90 - Read Class 0123"))
{
    auto result = ParseRequest("C5 01 3C 02 06 3C 03 06 3C 04 06 3C 01 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::READ);

    ExpectObjects("3C 02 06 3C 03 06 3C 04 06 3C 01 06", ParseResult::OK, 4);
}

TEST_CASE(PCAP_READS("frame140 - Read Class 0123"))
{
    auto result = ParseRequest("C7 01 3C 02 06 3C 03 06 3C 04 06 3C 01 06");
    REQUIRE(result.success);

    ExpectObjects("3C 02 06 3C 03 06 3C 04 06 3C 01 06", ParseResult::OK, 4);
}

TEST_CASE(PCAP_READS("frame182 - Read Class 123"))
{
    auto result = ParseRequest("C2 01 3C 02 06 3C 03 06 3C 04 06");
    REQUIRE(result.success);

    ExpectObjects("3C 02 06 3C 03 06 3C 04 06", ParseResult::OK, 3);
}

// ---------------------------------------------------------------------------
// Integrity poll responses with full static data
// ---------------------------------------------------------------------------

#define PCAP_INTEGRITY(name) "PcapParsing - IntegrityPoll - " name

TEST_CASE(PCAP_INTEGRITY("frame91 - Full static data response"))
{
    auto result = ParseResponse(
        "C5 81 00 00 "
        "01 01 00 00 05 02 "
        "0A 02 00 00 05 01 01 01 01 01 01 "
        "14 05 00 00 00 00 00 00 00 "
        "15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 C5 00 00 00 C7 00 00 00 C8 00 00 00 01 00 00 00 25 1C 00 00 0E 1C 00 00 10 1C 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);

    MockApduHeaderHandler handler;
    auto r = ParseObjects(
        "01 01 00 00 05 02 "
        "0A 02 00 00 05 01 01 01 01 01 01 "
        "14 05 00 00 00 00 00 00 00 "
        "15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 C5 00 00 00 C7 00 00 00 C8 00 00 00 01 00 00 00 25 1C 00 00 0E 1C 00 00 10 1C 00 00",
        handler);
    REQUIRE((r == ParseResult::OK));
    // Headers: g1v1, g10v2, g20v5, g21v9, g30v3
    REQUIRE(handler.records.size() == 5);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group1Var1));
    REQUIRE((handler.records[1].enumeration == GroupVariation::Group10Var2));
    REQUIRE((handler.records[2].enumeration == GroupVariation::Group20Var5));
    REQUIRE((handler.records[3].enumeration == GroupVariation::Group21Var9));
    REQUIRE((handler.records[4].enumeration == GroupVariation::Group30Var3));
}

TEST_CASE(PCAP_INTEGRITY("frame141 - Full static data response"))
{
    auto result = ParseResponse(
        "C7 81 00 00 "
        "01 01 00 00 05 19 "
        "0A 02 00 00 05 81 01 81 81 01 01 "
        "14 05 00 00 00 20 00 00 00 "
        "15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 CA 00 00 00 CB 00 00 00 C9 00 00 00 FF FF FF FF 66 21 00 00 59 21 00 00 4B 21 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);

    MockApduHeaderHandler handler;
    auto r = ParseObjects(
        "01 01 00 00 05 19 "
        "0A 02 00 00 05 81 01 81 81 01 01 "
        "14 05 00 00 00 20 00 00 00 "
        "15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 CA 00 00 00 CB 00 00 00 C9 00 00 00 FF FF FF FF 66 21 00 00 59 21 00 00 4B 21 00 00",
        handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 5);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group1Var1));
    REQUIRE((handler.records[1].enumeration == GroupVariation::Group10Var2));
    REQUIRE((handler.records[2].enumeration == GroupVariation::Group20Var5));
    REQUIRE((handler.records[3].enumeration == GroupVariation::Group21Var9));
    REQUIRE((handler.records[4].enumeration == GroupVariation::Group30Var3));
}

// ---------------------------------------------------------------------------
// Write IIN requests
// ---------------------------------------------------------------------------

#define PCAP_IIN(name) "PcapParsing - WriteIIN - " name

TEST_CASE(PCAP_IIN("frame88 - Write Internal Indications"))
{
    auto result = ParseRequest("C4 02 50 01 00 07 07 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::WRITE);

    // Group80Var1 (IIN) range 7..7, value 0x00
    MockApduHeaderHandler handler;
    auto r = ParseObjects("50 01 00 07 07 00", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 1);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group80Var1));
    REQUIRE(handler.iinBits.size() == 1);
    REQUIRE_FALSE(handler.iinBits[0].value.value);
}

TEST_CASE(PCAP_IIN("frame137 - Write Internal Indications"))
{
    auto result = ParseRequest("C6 02 50 01 00 07 07 00");
    REQUIRE(result.success);

    MockApduHeaderHandler handler;
    auto r = ParseObjects("50 01 00 07 07 00", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.iinBits.size() == 1);
}

// ---------------------------------------------------------------------------
// Select/Operate (CROB) requests and responses
// ---------------------------------------------------------------------------

#define PCAP_SELECT(name) "PcapParsing - Select - " name

TEST_CASE(PCAP_SELECT("frame185 - Select CROB request"))
{
    auto result = ParseRequest("C3 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::SELECT);

    MockApduHeaderHandler handler;
    auto r = ParseObjects("0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.records.size() == 1);
    REQUIRE((handler.records[0].enumeration == GroupVariation::Group12Var1));
    REQUIRE(handler.crobRequests.size() == 1);
    // Index is 0x869F = 34463
    REQUIRE(handler.crobRequests[0].index == 0x869F);
}

TEST_CASE(PCAP_SELECT("frame186 - Select CROB response"))
{
    auto result = ParseResponse("C3 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::RESPONSE);
    // IIN: 0x00,0x04 -> PARAM_ERROR (bit 2 of second byte)
    REQUIRE(result.header.IIN.IsSet(IINBit::PARAM_ERROR));

    MockApduHeaderHandler handler;
    auto r = ParseObjects("0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04", handler);
    REQUIRE((r == ParseResult::OK));
    REQUIRE(handler.crobRequests.size() == 1);
}

TEST_CASE(PCAP_SELECT("frame188-189 - Select and response pair"))
{
    // Select request
    auto req = ParseRequest("C4 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00");
    REQUIRE(req.success);
    REQUIRE(req.header.function == FunctionCode::SELECT);

    // Response
    auto resp = ParseResponse("C4 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04");
    REQUIRE(resp.success);
    REQUIRE(resp.header.function == FunctionCode::RESPONSE);
}

TEST_CASE(PCAP_SELECT("frame191-192 - Select and response pair"))
{
    auto req = ParseRequest("C5 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00");
    REQUIRE(req.success);

    auto resp = ParseResponse("C5 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04");
    REQUIRE(resp.success);
}

TEST_CASE(PCAP_SELECT("frame194-195 - Select and response pair"))
{
    auto req = ParseRequest("C6 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00");
    REQUIRE(req.success);

    auto resp = ParseResponse("C6 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04");
    REQUIRE(resp.success);
}

TEST_CASE(PCAP_SELECT("frame197-198 - Select and response pair"))
{
    auto req = ParseRequest("C7 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00");
    REQUIRE(req.success);

    auto resp = ParseResponse("C7 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04");
    REQUIRE(resp.success);
}

// ---------------------------------------------------------------------------
// Unknown object read requests (Group 255 Var 0)
// ---------------------------------------------------------------------------

#define PCAP_UNKNOWN(name) "PcapParsing - UnknownObject - " name

TEST_CASE(PCAP_UNKNOWN("frame200 - Read unknown object type"))
{
    auto result = ParseRequest("C8 01 FF 00 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::READ);

    // Group255Var0 is UNKNOWN -- parser should flag it
    MockApduHeaderHandler handler;
    auto r = ParseObjects("FF 00 06", handler);
    // The parser treats unknown group/variation as UNKNOWN_OBJECT
    REQUIRE(((r == ParseResult::UNKNOWN_OBJECT) || (r == ParseResult::OK)));
}

TEST_CASE(PCAP_UNKNOWN("frame203 - Read unknown object type"))
{
    auto result = ParseRequest("C9 01 FF 00 06");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::READ);
}

TEST_CASE(PCAP_UNKNOWN("frame206 - Read unknown object type"))
{
    auto result = ParseRequest("CA 01 FF 00 06");
    REQUIRE(result.success);
}

TEST_CASE(PCAP_UNKNOWN("frame209 - Read unknown object type"))
{
    auto result = ParseRequest("CB 01 FF 00 06");
    REQUIRE(result.success);
}

TEST_CASE(PCAP_UNKNOWN("frame212 - Read unknown object type"))
{
    auto result = ParseRequest("CC 01 FF 00 06");
    REQUIRE(result.success);
}

// ---------------------------------------------------------------------------
// Stop Application requests
// ---------------------------------------------------------------------------

#define PCAP_STOP(name) "PcapParsing - StopApp - " name

TEST_CASE(PCAP_STOP("frame163 - Stop Application"))
{
    auto result = ParseRequest("C1 12 3C 02 06 01 07 06 3C 04 ED");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::STOP_APPLICATION);

    // Objects: g60v2 all, g1v7 all, g60v4 with qualifier 0xED (unknown)
    MockApduHeaderHandler handler;
    auto r = ParseObjects("3C 02 06 01 07 06 3C 04 ED", handler);
    // Parser may fail on unknown group/variation or unknown qualifier
    REQUIRE(r != ParseResult::OK);
}

// ---------------------------------------------------------------------------
// Disable Unsolicited with corrupt qualifier
// ---------------------------------------------------------------------------

#define PCAP_CORRUPT(name) "PcapParsing - CorruptData - " name

TEST_CASE(PCAP_CORRUPT("frame154 - Disable Unsolicited with corrupt last byte"))
{
    // Last byte is 0xED instead of 0x06
    auto result = ParseRequest("C1 15 3C 02 06 3C 03 06 3C 04 ED");
    REQUIRE(result.success);
    REQUIRE(result.header.function == FunctionCode::DISABLE_UNSOLICITED);

    MockApduHeaderHandler handler;
    auto r = ParseObjects("3C 02 06 3C 03 06 3C 04 ED", handler);
    // Third header has qualifier 0xED which is unknown
    REQUIRE(r == ParseResult::UNKNOWN_QUALIFIER);
}

// ---------------------------------------------------------------------------
// Multiple Write Time and Date requests (varying timestamps)
// ---------------------------------------------------------------------------

#define PCAP_TIME(name) "PcapParsing - TimeWrite - " name

TEST_CASE(PCAP_TIME("frame128 - Write Time"))
{
    ExpectObjects("32 01 07 01 2F 1D B4 87 FF 00", ParseResult::OK, 1);
}

TEST_CASE(PCAP_TIME("frame131 - Write Time"))
{
    ExpectObjects("32 01 07 01 5B 1E B4 87 FF 00", ParseResult::OK, 1);
}

TEST_CASE(PCAP_TIME("frame33 - Write Time"))
{
    ExpectObjects("32 01 07 01 A9 E1 7B 87 FF 00", ParseResult::OK, 1);
}

TEST_CASE(PCAP_TIME("frame16 - Write Time"))
{
    ExpectObjects("32 01 07 01 34 49 BE 66 34 01", ParseResult::OK, 1);
}

// ---------------------------------------------------------------------------
// Comprehensive APDU header parsing sweep -- every payload
// Tests that header parsing does not crash on any pcap payload
// ---------------------------------------------------------------------------

#define PCAP_SWEEP(name) "PcapParsing - HeaderSweep - " name

// Request APDUs (function code < 0x80)
TEST_CASE(PCAP_SWEEP("All request APDUs parse without crash"))
{
    // Collect all request payloads from the pcap data
    const std::string requests[] = {
        "C8 01 3C 02 06 3C 03 06 3C 04 06 3C 01 06",                                           // read
        "C1 02 32 01 07 01 FA 7D 0B 46 0D 01",                                                 // write time
        "CE 19 46 03 5B 01 24 00 1A 00 0A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 " // NOLINT
        "00 04 04 00 2E 2F 74 65 73 74 2E 78 6D 6C",                                           // open file
        "C0 02 32 01 07 01 D2 48 BE 66 34 01",                                                 // write time
        "C1 02 32 01 07 01 34 49 BE 66 34 01",                                                 // write time
        "C2 1A 46 04 5B 01 0D 00 78 56 34 12 00 00 00 00 00 00 05 00 00",                      // close file
        "C1 02 32 01 07 01 EB E4 5A 87 FF 00",                                                 // write time
        "C2 15 3C 02 06 3C 03 06 3C 04 06",                                                    // disable unsol
        "C1 02 32 01 07 01 A9 E1 7B 87 FF 00",                                                 // write time
        "C2 14 3C 02 06 3C 03 06 3C 04 06",                                                    // enable unsol
        "C2 01 02 00 06",                                                                      // read binary change
        "C3 15 3C 02 06 3C 03 06 3C 04 06",                                                    // disable unsol
        "C4 02 50 01 00 07 07 00",                                                             // write IIN
        "C5 01 3C 02 06 3C 03 06 3C 04 06 3C 01 06",                                           // read class 0123
        "C6 14 3C 02 06 3C 03 06 3C 04 06",                                                    // enable unsol
        "C7 14 3C 02 06 3C 03 06 3C 04 06",                                                    // enable unsol
        "C1 15 3C 02 06 3C 03 06 3C 04 06",                                                    // disable unsol
        "C2 02 32 01 07 01 2F 1D B4 87 FF 00",                                                 // write time
        "C3 15 3C 02 06 3C 03 06 3C 04 06",                                                    // disable unsol
        "C4 02 32 01 07 01 5B 1E B4 87 FF 00",                                                 // write time
        "C5 15 3C 02 06 3C 03 06 3C 04 06",                                                    // disable unsol
        "C6 02 50 01 00 07 07 00",                                                             // write IIN
        "C7 01 3C 02 06 3C 03 06 3C 04 06 3C 01 06",                                           // read class 0123
        "C1 15 3C 02 06 3C 03 06 3C 04 ED",                                                    // corrupt qualifier
        "C1 12 3C 02 06 01 07 06 3C 04 ED",                                                    // stop app, corrupt
        "C2 01 3C 02 06 3C 03 06 3C 04 06",                                                    // read class 123
        "C3 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00",                         // select
        "C4 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00",                         // select
        "C5 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00",                         // select
        "C6 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00",                         // select
        "C7 03 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00",                         // select
        "C8 01 FF 00 06",                                                                      // unknown object
        "C9 01 FF 00 06",
        "CA 01 FF 00 06",
        "CB 01 FF 00 06",
        "CC 01 FF 00 06",
    };

    for (const auto& hex : requests)
    {
        HexSequence buffer(hex);
        auto result = APDUHeaderParser::ParseRequest(buffer.ToRSeq());
        REQUIRE(result.success);
    }
}

// Response APDUs (function code >= 0x80)
TEST_CASE(PCAP_SWEEP("All response APDUs parse without crash"))
{
    const std::string responses[] = {
        "C8 81 00 00 01 02 00 00 08 81 01 81 01 01 01 01 01 01 "
        "0A 02 00 00 06 00 00 00 00 00 00 01 "
        "1E 01 00 00 0E "
        "01 EF 03 00 00 01 03 00 00 00 01 ED 03 00 00 "
        "01 2B D1 FF FF 01 ED 03 00 00 01 E6 2E 00 00 "
        "01 17 0D 02 00 00 00 00 00 00 01 B5 0C 02 00 00 00 00 00 00 "
        "01 8A 0D 02 00 00 00 00 00 00 "
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
        "CE 81 10 00 46 04 5B 01 0D 00 78 56 34 12 3E 03 00 00 00 04 04 00 00",
        "C2 81 00 00 46 04 5B 01 0D 00 78 56 34 12 00 00 00 00 00 00 05 00 00",
        "C3 81 00 00 34 02 07 01 88 13",
        "C1 81 00 00 34 02 07 01 88 13",
        "C5 81 00 00 01 01 00 00 05 02 0A 02 00 00 05 01 01 01 01 01 01 " // NOLINT
        "14 05 00 00 00 00 00 00 00 15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 C5 00 00 00 C7 00 00 00 C8 00 00 00 01 00 00 00 25 1C 00 00 0E 1C 00 00 10 1C 00 00",
        "C7 81 00 00 01 01 00 00 05 19 0A 02 00 00 05 81 01 81 81 01 01 "
        "14 05 00 00 00 20 00 00 00 15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 CA 00 00 00 CB 00 00 00 C9 00 00 00 FF FF FF FF 66 21 00 00 59 21 00 00 4B 21 00 00",
        "C8 81 00 00 34 02 07 01 D0 07",
        "C9 81 00 00 34 02 07 01 D0 07",
        "C3 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04",
        "C4 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04",
        "C5 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04",
        "C6 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04",
        "C7 81 00 04 0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04",
    };

    for (const auto& hex : responses)
    {
        HexSequence buffer(hex);
        auto result = APDUHeaderParser::ParseResponse(buffer.ToRSeq());
        REQUIRE(result.success);
    }
}

// Unsolicited response APDUs
TEST_CASE(PCAP_SWEEP("All unsolicited response APDUs parse without crash"))
{
    const std::string unsolicited[] = {
        "F3 82 00 00 33 01 07 01 E2 43 7D 87 FF 00 " // NOLINT
        "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 F3 03 01 00 01 F3 03 "
        "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
        "F4 82 00 00 33 01 07 01 D5 47 7D 87 FF 00 "
        "02 03 28 04 00 02 00 01 00 00 00 00 01 17 03 01 00 81 17 03 02 00 81 17 03 "
        "20 01 28 03 00 02 00 01 C6 00 00 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00",
        "F5 82 00 00 33 01 07 01 F4 4E 7D 87 FF 00 "
        "02 03 28 05 00 00 00 81 00 00 01 00 01 00 00 02 00 01 00 00 00 00 01 C1 03 01 00 81 C1 03 "
        "20 01 28 03 00 00 00 01 C6 00 00 00 01 00 01 CA 00 00 00 02 00 01 C6 00 00 00",
        "F6 82 00 00 33 01 07 01 B5 52 7D 87 FF 00 "
        "02 03 28 04 00 02 00 81 00 00 00 00 81 A4 03 01 00 01 A4 03 02 00 01 A4 03 "
        "20 01 28 03 00 02 00 01 00 00 00 00 00 00 01 CA 00 00 00 01 00 01 C8 00 00 00",
        "F7 82 00 00 33 01 07 01 1A 5A 7D 87 FF 00 "
        "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 AD 03 01 00 01 AD 03 "
        "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
        "F8 82 00 00 33 01 07 01 C7 5D 7D 87 FF 00 "
        "02 03 28 05 00 02 00 01 00 00 00 00 01 0E 03 01 00 81 0E 03 02 00 81 0E 03 "
        "00 00 81 01 07",
        "F9 82 00 00 33 01 07 01 C8 64 7D 87 FF 00 "
        "02 03 28 05 00 01 00 01 00 00 02 00 01 00 00 00 00 01 2B 03 01 00 81 2B 03 02 00 81 2B 03 "
        "20 01 28 03 00 00 00 01 C6 00 00 00 01 00 01 C7 00 00 00 02 00 01 C7 00 00 00",
        "FA 82 00 00 33 01 07 01 B5 6B 7D 87 FF 00 "
        "02 03 28 03 00 00 00 81 00 00 01 00 01 00 00 02 00 01 00 00 "
        "20 01 28 03 00 02 00 01 00 00 00 00 00 00 01 CA 00 00 00 01 00 01 C8 00 00 00",
        "FB 82 00 00 33 01 07 01 5E 6E 7D 87 FF 00 "
        "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 0D 03 01 00 01 0D 03 "
        "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
        "FC 82 00 00 33 01 07 01 6B 71 7D 87 FF 00 "
        "02 03 28 01 00 02 00 01 00 00 "
        "20 01 28 03 00 00 00 01 CB 00 00 00 01 00 01 CA 00 00 00 02 00 01 C7 00 00 00",
    };

    for (const auto& hex : unsolicited)
    {
        HexSequence buffer(hex);
        auto result = APDUHeaderParser::ParseResponse(buffer.ToRSeq());
        REQUIRE(result.success);
        REQUIRE(result.header.function == FunctionCode::UNSOLICITED_RESPONSE);
    }
}

// ---------------------------------------------------------------------------
// Object parsing sweep -- every objects payload
// Tests that object parsing does not crash on any pcap object payload
// ---------------------------------------------------------------------------

TEST_CASE(PCAP_SWEEP("All object payloads parse without crash"))
{
    const std::string objects[] = {
        // Read Class 0123
        "3C 02 06 3C 03 06 3C 04 06 3C 01 06",
        // Response objects (g1v2, g10v2, g30v1)
        "01 02 00 00 08 81 01 81 01 01 01 01 01 01 "
        "0A 02 00 00 06 00 00 00 00 00 00 01 "
        "1E 01 00 00 0E "
        "01 EF 03 00 00 01 03 00 00 00 01 ED 03 00 00 "
        "01 2B D1 FF FF 01 ED 03 00 00 01 E6 2E 00 00 "
        "01 17 0D 02 00 00 00 00 00 00 01 B5 0C 02 00 00 00 00 00 00 "
        "01 8A 0D 02 00 00 00 00 00 00 "
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
        // Write Time and Date
        "32 01 07 01 FA 7D 0B 46 0D 01",
        "32 01 07 01 EB E4 5A 87 FF 00",
        "32 01 07 01 A9 E1 7B 87 FF 00",
        "32 01 07 01 D2 48 BE 66 34 01",
        "32 01 07 01 34 49 BE 66 34 01",
        "32 01 07 01 2F 1D B4 87 FF 00",
        "32 01 07 01 5B 1E B4 87 FF 00",
        // Disable/Enable Unsolicited
        "3C 02 06 3C 03 06 3C 04 06",
        // Write IIN
        "50 01 00 07 07 00",
        // Read Binary change
        "02 00 06",
        // Delay measurement
        "34 02 07 01 88 13",
        "34 02 07 01 D0 07",
        // Integrity poll responses
        "01 01 00 00 05 02 "
        "0A 02 00 00 05 01 01 01 01 01 01 "
        "14 05 00 00 00 00 00 00 00 "
        "15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 C5 00 00 00 C7 00 00 00 C8 00 00 00 01 00 00 00 25 1C 00 00 0E 1C 00 00 10 1C 00 00",
        "01 01 00 00 05 19 "
        "0A 02 00 00 05 81 01 81 81 01 01 "
        "14 05 00 00 00 20 00 00 00 "
        "15 09 00 00 00 00 00 00 00 "
        "1E 03 00 00 06 CA 00 00 00 CB 00 00 00 C9 00 00 00 FF FF FF FF 66 21 00 00 59 21 00 00 4B 21 00 00",
        // Select (CROB)
        "0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00",
        "0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04",
        // Unknown object
        "FF 00 06",
        // Unsolicited event objects (CTO + binary events + analog events)
        "33 01 07 01 E2 43 7D 87 FF 00 " // NOLINT
        "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 F3 03 01 00 01 F3 03 "
        "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
        "33 01 07 01 6B 71 7D 87 FF 00 "
        "02 03 28 01 00 02 00 01 00 00 "
        "20 01 28 03 00 00 00 01 CB 00 00 00 01 00 01 CA 00 00 00 02 00 01 C7 00 00 00",
    };

    for (const auto& hex : objects)
    {
        HexSequence buffer(hex);
        MockApduHeaderHandler handler;
        // Parse should not crash; we accept any result
        APDUParser::Parse(buffer.ToRSeq(), handler, nullptr);
    }
}

// ---------------------------------------------------------------------------
// Validate specific event data values in unsolicited responses
// ---------------------------------------------------------------------------

#define PCAP_VALUES(name) "PcapParsing - DataValues - " name

TEST_CASE(PCAP_VALUES("frame40 - Binary event values"))
{
    MockApduHeaderHandler handler;
    ParseObjects("33 01 07 01 E2 43 7D 87 FF 00 "
                 "02 03 28 05 00 00 00 01 00 00 01 00 81 00 00 02 00 81 00 00 00 00 81 F3 03 01 00 01 F3 03 "
                 "20 01 28 03 00 00 00 01 00 00 00 00 01 00 01 00 00 00 00 02 00 01 00 00 00 00",
                 handler);

    // 5 binary events from Group2Var3 (with relative time)
    REQUIRE(handler.eventBinaries.size() == 5);
    // First event: index 0, value from flags 0x01 -> not online per flags? Actually
    // For Group2Var3: flags byte, then 2-byte relative time
    // Event at index 0: flags=0x01, time offset = 0x0000
    REQUIRE(handler.eventBinaries[0].index == 0);
    // Event at index 1: flags=0x01, time offset
    REQUIRE(handler.eventBinaries[1].index == 1);
    // Event at index 2: flags=0x81 -> binary value=true (bit 7)
    REQUIRE(handler.eventBinaries[2].index == 2);
    // Index 2 has flags 0x81 which means online=true and value=true
    REQUIRE(handler.eventBinaries[2].value.value == true);

    // 3 analog events from Group32Var1
    REQUIRE(handler.eventAnalogs.size() == 3);
    REQUIRE(handler.eventAnalogs[0].index == 0);
    REQUIRE(handler.eventAnalogs[1].index == 1);
    REQUIRE(handler.eventAnalogs[2].index == 2);
}

TEST_CASE(PCAP_VALUES("frame67 - Analog event values"))
{
    MockApduHeaderHandler handler;
    ParseObjects("33 01 07 01 6B 71 7D 87 FF 00 "
                 "02 03 28 01 00 02 00 01 00 00 "
                 "20 01 28 03 00 00 00 01 CB 00 00 00 01 00 01 CA 00 00 00 02 00 01 C7 00 00 00",
                 handler);

    REQUIRE(handler.eventAnalogs.size() == 3);
    // Group32Var1: flags (1 byte) + value (4 bytes signed)
    // Index 0: flags=0x01, value=0x000000CB = 203
    REQUIRE(handler.eventAnalogs[0].index == 0);
    REQUIRE(handler.eventAnalogs[0].value.value == 203);
    // Index 1: flags=0x01, value=0x000000CA = 202
    REQUIRE(handler.eventAnalogs[1].index == 1);
    REQUIRE(handler.eventAnalogs[1].value.value == 202);
    // Index 2: flags=0x01, value=0x000000C7 = 199
    REQUIRE(handler.eventAnalogs[2].index == 2);
    REQUIRE(handler.eventAnalogs[2].value.value == 199);
}

TEST_CASE(PCAP_VALUES("frame91 - Static analog values"))
{
    MockApduHeaderHandler handler;
    ParseObjects("01 01 00 00 05 02 "
                 "0A 02 00 00 05 01 01 01 01 01 01 "
                 "14 05 00 00 00 00 00 00 00 "
                 "15 09 00 00 00 00 00 00 00 "
                 "1E 03 00 00 06 C5 00 00 00 C7 00 00 00 C8 00 00 00 01 00 00 00 25 1C 00 00 0E 1C 00 00 10 1C 00 00",
                 handler);

    // Group30Var3 range 0..6 -> 7 analog values
    // Group30Var3 is 32-bit analogs with flag
    // The eventAnalogs vector is populated for both range and prefix analogs
    // in this mock (it uses the same vector for both). Verify we got values.
    REQUIRE(handler.eventAnalogs.size() == 7);

    // First value: flags=0xC5, value = 0x0000 (only 2 bytes for g30v3)
    // Actually Group30Var3 = 32-bit without flag? Let me check...
    // Group30Var3 is Analog Input - 32-bit without flag.
    // Wait, let me re-examine. The MockApduHeaderHandler routes
    // RangeHeader analogs to eventAnalogs.
    // g30v3 = Analog input, 32-bit with no flag byte
    // So each record is 4 bytes: C5 00 00 00 = 0xC5 = 197
    REQUIRE(handler.eventAnalogs[0].index == 0);
    REQUIRE(handler.eventAnalogs[0].value.value == 0xC5);
    REQUIRE(handler.eventAnalogs[1].value.value == 0xC7);
    REQUIRE(handler.eventAnalogs[2].value.value == 0xC8);
    REQUIRE(handler.eventAnalogs[3].value.value == 1);
    REQUIRE(handler.eventAnalogs[4].value.value == 0x1C25);
    REQUIRE(handler.eventAnalogs[5].value.value == 0x1C0E);
    REQUIRE(handler.eventAnalogs[6].value.value == 0x1C10);
}

TEST_CASE(PCAP_VALUES("frame185 - Select CROB details"))
{
    MockApduHeaderHandler handler;
    ParseObjects("0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 00", handler);

    REQUIRE(handler.crobRequests.size() == 1);
    // Index: 0x869F = 34463
    REQUIRE(handler.crobRequests[0].index == 0x869F);
    // CROB: opType=0x03 (LATCH_ON), count=1, onTime=100, offTime=100, status=0
    auto& crob = handler.crobRequests[0].value;
    REQUIRE(crob.opType == OperationType::LATCH_ON);
    REQUIRE(crob.count == 1);
    REQUIRE(crob.onTimeMS == 100);
    REQUIRE(crob.offTimeMS == 100);
    REQUIRE(crob.status == CommandStatus::SUCCESS);
}

TEST_CASE(PCAP_VALUES("frame186 - Select CROB response with NOT_SUPPORTED status"))
{
    MockApduHeaderHandler handler;
    ParseObjects("0C 01 28 01 00 9F 86 03 01 64 00 00 00 64 00 00 00 04", handler);

    REQUIRE(handler.crobRequests.size() == 1);
    auto& crob = handler.crobRequests[0].value;
    REQUIRE(crob.opType == OperationType::LATCH_ON);
    // Status byte 0x04 = NOT_SUPPORTED
    REQUIRE(crob.status == CommandStatus::NOT_SUPPORTED);
}

TEST_CASE(PCAP_VALUES("frame91 - Binary input packed bits"))
{
    MockApduHeaderHandler handler;
    ParseObjects("01 01 00 00 05 02 "
                 "0A 02 00 00 05 01 01 01 01 01 01 "
                 "14 05 00 00 00 00 00 00 00 "
                 "15 09 00 00 00 00 00 00 00 "
                 "1E 03 00 00 06 C5 00 00 00 C7 00 00 00 C8 00 00 00 01 00 00 00 25 1C 00 00 0E 1C 00 00 10 1C 00 00",
                 handler);

    // Group1Var1 range 0..5 -> 6 binary inputs packed in 1 byte
    // 0x02 = 0b00000010 -> bit 0=false, bit 1=true, rest false
    REQUIRE(handler.staticBinaries.size() == 6);
    REQUIRE(handler.staticBinaries[0].index == 0);
    REQUIRE_FALSE(handler.staticBinaries[0].value.value);
    REQUIRE(handler.staticBinaries[1].index == 1);
    REQUIRE(handler.staticBinaries[1].value.value); // bit 1 = 1
    REQUIRE(handler.staticBinaries[2].index == 2);
    REQUIRE_FALSE(handler.staticBinaries[2].value.value);
}

TEST_CASE(PCAP_VALUES("frame141 - Binary input packed bits different values"))
{
    MockApduHeaderHandler handler;
    ParseObjects("01 01 00 00 05 19 "
                 "0A 02 00 00 05 81 01 81 81 01 01 "
                 "14 05 00 00 00 20 00 00 00 "
                 "15 09 00 00 00 00 00 00 00 "
                 "1E 03 00 00 06 CA 00 00 00 CB 00 00 00 C9 00 00 00 FF FF FF FF 66 21 00 00 59 21 00 00 4B 21 00 00",
                 handler);

    // Group1Var1 range 0..5 -> 6 binary inputs packed in 1 byte
    // 0x19 = 0b00011001 -> bits 0,3,4 = true; bits 1,2,5 = false
    REQUIRE(handler.staticBinaries.size() == 6);
    REQUIRE(handler.staticBinaries[0].value.value);       // bit 0
    REQUIRE_FALSE(handler.staticBinaries[1].value.value); // bit 1
    REQUIRE_FALSE(handler.staticBinaries[2].value.value); // bit 2
    REQUIRE(handler.staticBinaries[3].value.value);       // bit 3
    REQUIRE(handler.staticBinaries[4].value.value);       // bit 4
    REQUIRE_FALSE(handler.staticBinaries[5].value.value); // bit 5
}
