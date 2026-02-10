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

#include "Group21.h"

#include "app/MeasurementFactory.h"
#include "app/WriteConversions.h"
#include "app/parsing/DNPTimeParsing.h"

#include <ser4cpp/serialization/LittleEndian.h>

using namespace ser4cpp;

namespace opendnp3
{

// ------- Group21Var1 -------

Group21Var1::Group21Var1() : flags(0), value(0) {}

bool Group21Var1::Read(rseq_t& buffer, Group21Var1& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group21Var1::Write(const Group21Var1& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group21Var1::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var1 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var1::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var1::Write(ConvertGroup21Var1::Apply(value), buff);
}

// ------- Group21Var2 -------

Group21Var2::Group21Var2() : flags(0), value(0) {}

bool Group21Var2::Read(rseq_t& buffer, Group21Var2& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group21Var2::Write(const Group21Var2& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group21Var2::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var2 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var2::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var2::Write(ConvertGroup21Var2::Apply(value), buff);
}

// ------- Group21Var3 -------

Group21Var3::Group21Var3() : flags(0), value(0) {}

bool Group21Var3::Read(rseq_t& buffer, Group21Var3& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group21Var3::Write(const Group21Var3& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group21Var3::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var3 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var3::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var3::Write(ConvertGroup21Var3::Apply(value), buff);
}

// ------- Group21Var4 -------

Group21Var4::Group21Var4() : flags(0), value(0) {}

bool Group21Var4::Read(rseq_t& buffer, Group21Var4& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group21Var4::Write(const Group21Var4& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group21Var4::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var4 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var4::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var4::Write(ConvertGroup21Var4::Apply(value), buff);
}

// ------- Group21Var5 -------

Group21Var5::Group21Var5() : flags(0), value(0), time(0) {}

bool Group21Var5::Read(rseq_t& buffer, Group21Var5& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group21Var5::Write(const Group21Var5& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group21Var5::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var5 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var5::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var5::Write(ConvertGroup21Var5::Apply(value), buff);
}

// ------- Group21Var6 -------

Group21Var6::Group21Var6() : flags(0), value(0), time(0) {}

bool Group21Var6::Read(rseq_t& buffer, Group21Var6& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group21Var6::Write(const Group21Var6& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group21Var6::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var6 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var6::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var6::Write(ConvertGroup21Var6::Apply(value), buff);
}

// ------- Group21Var7 -------

Group21Var7::Group21Var7() : flags(0), value(0), time(0) {}

bool Group21Var7::Read(rseq_t& buffer, Group21Var7& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group21Var7::Write(const Group21Var7& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group21Var7::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var7 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var7::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var7::Write(ConvertGroup21Var7::Apply(value), buff);
}

// ------- Group21Var8 -------

Group21Var8::Group21Var8() : flags(0), value(0), time(0) {}

bool Group21Var8::Read(rseq_t& buffer, Group21Var8& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group21Var8::Write(const Group21Var8& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group21Var8::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var8 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var8::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var8::Write(ConvertGroup21Var8::Apply(value), buff);
}

// ------- Group21Var9 -------

Group21Var9::Group21Var9() : value(0) {}

bool Group21Var9::Read(rseq_t& buffer, Group21Var9& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group21Var9::Write(const Group21Var9& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group21Var9::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var9 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var9::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var9::Write(ConvertGroup21Var9::Apply(value), buff);
}

// ------- Group21Var10 -------

Group21Var10::Group21Var10() : value(0) {}

bool Group21Var10::Read(rseq_t& buffer, Group21Var10& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group21Var10::Write(const Group21Var10& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group21Var10::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var10 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var10::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var10::Write(ConvertGroup21Var10::Apply(value), buff);
}

// ------- Group21Var11 -------

Group21Var11::Group21Var11() : value(0) {}

bool Group21Var11::Read(rseq_t& buffer, Group21Var11& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group21Var11::Write(const Group21Var11& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group21Var11::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var11 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var11::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var11::Write(ConvertGroup21Var11::Apply(value), buff);
}

// ------- Group21Var12 -------

Group21Var12::Group21Var12() : value(0) {}

bool Group21Var12::Read(rseq_t& buffer, Group21Var12& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group21Var12::Write(const Group21Var12& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group21Var12::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group21Var12 value;
    if (Read(buff, value))
    {
        output = FrozenCounterFactory::From(value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group21Var12::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group21Var12::Write(ConvertGroup21Var12::Apply(value), buff);
}

} // namespace opendnp3
