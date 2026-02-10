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

#ifndef OPENDNP3_GROUP21_H
#define OPENDNP3_GROUP21_H

#include "app/DNP3Serializer.h"
#include "app/MeasurementTypeSpecs.h"

#include "opendnp3/app/DNPTime.h"
#include "opendnp3/app/GroupVariationID.h"

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

// Frozen Counter - Any Variation
struct Group21Var0
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 0);
    }
};

// Frozen Counter - 32-bit With Flag
struct Group21Var1
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 1);
    }

    Group21Var1();

    static size_t Size()
    {
        return 5;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var1&);
    static bool Write(const Group21Var1&, ser4cpp::wseq_t&);

    typedef uint32_t ValueType;
    uint8_t flags;
    uint32_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
    static const StaticFrozenCounterVariation svariation = StaticFrozenCounterVariation::Group21Var1;
};

// Frozen Counter - 16-bit With Flag
struct Group21Var2
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 2);
    }

    Group21Var2();

    static size_t Size()
    {
        return 3;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var2&);
    static bool Write(const Group21Var2&, ser4cpp::wseq_t&);

    typedef uint16_t ValueType;
    uint8_t flags;
    uint16_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
    static const StaticFrozenCounterVariation svariation = StaticFrozenCounterVariation::Group21Var2;
};

// Frozen Delta Counter - 32-bit With Flag
struct Group21Var3
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 3);
    }

    Group21Var3();

    static size_t Size()
    {
        return 5;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var3&);
    static bool Write(const Group21Var3&, ser4cpp::wseq_t&);

    typedef uint32_t ValueType;
    uint8_t flags;
    uint32_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

// Frozen Delta Counter - 16-bit With Flag
struct Group21Var4
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 4);
    }

    Group21Var4();

    static size_t Size()
    {
        return 3;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var4&);
    static bool Write(const Group21Var4&, ser4cpp::wseq_t&);

    typedef uint16_t ValueType;
    uint8_t flags;
    uint16_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

// Frozen Counter - 32-bit With Flag and Time
struct Group21Var5
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 5);
    }

    Group21Var5();

    static size_t Size()
    {
        return 11;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var5&);
    static bool Write(const Group21Var5&, ser4cpp::wseq_t&);

    typedef uint32_t ValueType;
    uint8_t flags;
    uint32_t value;
    DNPTime time;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
    static const StaticFrozenCounterVariation svariation = StaticFrozenCounterVariation::Group21Var5;
};

// Frozen Counter - 16-bit With Flag and Time
struct Group21Var6
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 6);
    }

    Group21Var6();

    static size_t Size()
    {
        return 9;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var6&);
    static bool Write(const Group21Var6&, ser4cpp::wseq_t&);

    typedef uint16_t ValueType;
    uint8_t flags;
    uint16_t value;
    DNPTime time;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
    static const StaticFrozenCounterVariation svariation = StaticFrozenCounterVariation::Group21Var6;
};

// Frozen Delta Counter - 32-bit With Flag and Time
struct Group21Var7
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 7);
    }

    Group21Var7();

    static size_t Size()
    {
        return 11;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var7&);
    static bool Write(const Group21Var7&, ser4cpp::wseq_t&);

    typedef uint32_t ValueType;
    uint8_t flags;
    uint32_t value;
    DNPTime time;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

// Frozen Delta Counter - 16-bit With Flag and Time
struct Group21Var8
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 8);
    }

    Group21Var8();

    static size_t Size()
    {
        return 9;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var8&);
    static bool Write(const Group21Var8&, ser4cpp::wseq_t&);

    typedef uint16_t ValueType;
    uint8_t flags;
    uint16_t value;
    DNPTime time;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

// Frozen Counter - 32-bit Without Flag
struct Group21Var9
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 9);
    }

    Group21Var9();

    static size_t Size()
    {
        return 4;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var9&);
    static bool Write(const Group21Var9&, ser4cpp::wseq_t&);

    typedef uint32_t ValueType;
    uint32_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
    static const StaticFrozenCounterVariation svariation = StaticFrozenCounterVariation::Group21Var9;
};

// Frozen Counter - 16-bit Without Flag
struct Group21Var10
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 10);
    }

    Group21Var10();

    static size_t Size()
    {
        return 2;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var10&);
    static bool Write(const Group21Var10&, ser4cpp::wseq_t&);

    typedef uint16_t ValueType;
    uint16_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
    static const StaticFrozenCounterVariation svariation = StaticFrozenCounterVariation::Group21Var10;
};

// Frozen Delta Counter - 32-bit Without Flag
struct Group21Var11
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 11);
    }

    Group21Var11();

    static size_t Size()
    {
        return 4;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var11&);
    static bool Write(const Group21Var11&, ser4cpp::wseq_t&);

    typedef uint32_t ValueType;
    uint32_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

// Frozen Delta Counter - 16-bit Without Flag
struct Group21Var12
{
    static GroupVariationID ID()
    {
        return GroupVariationID(21, 12);
    }

    Group21Var12();

    static size_t Size()
    {
        return 2;
    }
    static bool Read(ser4cpp::rseq_t&, Group21Var12&);
    static bool Write(const Group21Var12&, ser4cpp::wseq_t&);

    typedef uint16_t ValueType;
    uint16_t value;

    typedef FrozenCounter Target;
    typedef FrozenCounterSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, FrozenCounter&);
    static bool WriteTarget(const FrozenCounter&, ser4cpp::wseq_t&);
    static DNP3Serializer<FrozenCounter> Inst()
    {
        return DNP3Serializer<FrozenCounter>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

} // namespace opendnp3

#endif
