//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
//
// This file is auto-generated. Do not edit manually
//
// Copyright 2013-2022 Step Function I/O, LLC
// Modified 2024-2026 f0rw4rd (experimental fork)
//
// Licensed to Green Energy Corp (www.greenenergycorp.com) and Step Function I/O
// LLC (https://stepfunc.io) under one or more contributor license agreements.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership. Green Energy Corp and Step Function I/O LLC license
// this file to you under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may obtain
// a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "opendnp3/gen/QualifierCode.h"

#include <stdexcept>

namespace opendnp3
{

uint8_t QualifierCodeSpec::to_type(QualifierCode arg)
{
    return static_cast<uint8_t>(arg);
}

QualifierCode QualifierCodeSpec::from_type(uint8_t arg)
{
    switch (arg)
    {
    case (0x0):
        return QualifierCode::UINT8_START_STOP;
    case (0x1):
        return QualifierCode::UINT16_START_STOP;
    case (0x2):
        return QualifierCode::UINT32_START_STOP;
    case (0x3):
        return QualifierCode::UINT8_ADDR;
    case (0x4):
        return QualifierCode::UINT16_ADDR;
    case (0x5):
        return QualifierCode::UINT32_ADDR;
    case (0x6):
        return QualifierCode::ALL_OBJECTS;
    case (0x7):
        return QualifierCode::UINT8_CNT;
    case (0x8):
        return QualifierCode::UINT16_CNT;
    case (0x9):
        return QualifierCode::UINT32_CNT;
    case (0x17):
        return QualifierCode::UINT8_CNT_UINT8_INDEX;
    case (0x28):
        return QualifierCode::UINT16_CNT_UINT16_INDEX;
    case (0x37):
        return QualifierCode::UINT32_CNT_UINT8_INDEX;
    case (0x5B):
        return QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT;
    default:
        return QualifierCode::UNDEFINED;
    }
}

char const* QualifierCodeSpec::to_string(QualifierCode arg)
{
    switch (arg)
    {
    case (QualifierCode::UINT8_START_STOP):
        return "UINT8_START_STOP";
    case (QualifierCode::UINT16_START_STOP):
        return "UINT16_START_STOP";
    case (QualifierCode::UINT32_START_STOP):
        return "UINT32_START_STOP";
    case (QualifierCode::UINT8_ADDR):
        return "UINT8_ADDR";
    case (QualifierCode::UINT16_ADDR):
        return "UINT16_ADDR";
    case (QualifierCode::UINT32_ADDR):
        return "UINT32_ADDR";
    case (QualifierCode::ALL_OBJECTS):
        return "ALL_OBJECTS";
    case (QualifierCode::UINT8_CNT):
        return "UINT8_CNT";
    case (QualifierCode::UINT16_CNT):
        return "UINT16_CNT";
    case (QualifierCode::UINT32_CNT):
        return "UINT32_CNT";
    case (QualifierCode::UINT8_CNT_UINT8_INDEX):
        return "UINT8_CNT_UINT8_INDEX";
    case (QualifierCode::UINT16_CNT_UINT16_INDEX):
        return "UINT16_CNT_UINT16_INDEX";
    case (QualifierCode::UINT32_CNT_UINT8_INDEX):
        return "UINT32_CNT_UINT8_INDEX";
    case (QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT):
        return "UINT8_CNT_UINT16_FREE_FORMAT";
    default:
        return "UNDEFINED";
    }
}

char const* QualifierCodeSpec::to_human_string(QualifierCode arg)
{
    switch (arg)
    {
    case (QualifierCode::UINT8_START_STOP):
        return "8-bit start stop";
    case (QualifierCode::UINT16_START_STOP):
        return "16-bit start stop";
    case (QualifierCode::UINT32_START_STOP):
        return "32-bit start stop";
    case (QualifierCode::UINT8_ADDR):
        return "8-bit address";
    case (QualifierCode::UINT16_ADDR):
        return "16-bit address";
    case (QualifierCode::UINT32_ADDR):
        return "32-bit address";
    case (QualifierCode::ALL_OBJECTS):
        return "all objects";
    case (QualifierCode::UINT8_CNT):
        return "8-bit count";
    case (QualifierCode::UINT16_CNT):
        return "16-bit count";
    case (QualifierCode::UINT32_CNT):
        return "32-bit count";
    case (QualifierCode::UINT8_CNT_UINT8_INDEX):
        return "8-bit count and prefix";
    case (QualifierCode::UINT16_CNT_UINT16_INDEX):
        return "16-bit count and prefix";
    case (QualifierCode::UINT32_CNT_UINT8_INDEX):
        return "32-bit count, 8-bit index";
    case (QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT):
        return "8-bit count, 16-bit free format";
    default:
        return "unknown";
    }
}

QualifierCode QualifierCodeSpec::from_string(const std::string& arg)
{
    if (arg == "UINT8_START_STOP")
        return QualifierCode::UINT8_START_STOP;
    if (arg == "UINT16_START_STOP")
        return QualifierCode::UINT16_START_STOP;
    if (arg == "UINT32_START_STOP")
        return QualifierCode::UINT32_START_STOP;
    if (arg == "UINT8_ADDR")
        return QualifierCode::UINT8_ADDR;
    if (arg == "UINT16_ADDR")
        return QualifierCode::UINT16_ADDR;
    if (arg == "UINT32_ADDR")
        return QualifierCode::UINT32_ADDR;
    if (arg == "ALL_OBJECTS")
        return QualifierCode::ALL_OBJECTS;
    if (arg == "UINT8_CNT")
        return QualifierCode::UINT8_CNT;
    if (arg == "UINT16_CNT")
        return QualifierCode::UINT16_CNT;
    if (arg == "UINT32_CNT")
        return QualifierCode::UINT32_CNT;
    if (arg == "UINT8_CNT_UINT8_INDEX")
        return QualifierCode::UINT8_CNT_UINT8_INDEX;
    if (arg == "UINT16_CNT_UINT16_INDEX")
        return QualifierCode::UINT16_CNT_UINT16_INDEX;
    if (arg == "UINT32_CNT_UINT8_INDEX")
        return QualifierCode::UINT32_CNT_UINT8_INDEX;
    if (arg == "UINT8_CNT_UINT16_FREE_FORMAT")
        return QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT;
    else
        return QualifierCode::UNDEFINED;
}

} // namespace opendnp3
