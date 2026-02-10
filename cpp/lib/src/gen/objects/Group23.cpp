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

#include "Group23.h"

#include "app/MeasurementFactory.h"
#include "app/WriteConversions.h"
#include "app/parsing/DNPTimeParsing.h"

#include <ser4cpp/serialization/LittleEndian.h>

using namespace ser4cpp;

namespace opendnp3
{

// ------- Group23Var1 -------

Group23Var1::Group23Var1() : flags(0), value(0) {}

bool Group23Var1::Read(rseq_t& buffer, Group23Var1& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group23Var1::Write(const Group23Var1& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group23Var1::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var1 value;
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

bool Group23Var1::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var1::Write(ConvertGroup23Var1::Apply(value), buff);
}

// ------- Group23Var2 -------

Group23Var2::Group23Var2() : flags(0), value(0) {}

bool Group23Var2::Read(rseq_t& buffer, Group23Var2& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group23Var2::Write(const Group23Var2& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group23Var2::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var2 value;
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

bool Group23Var2::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var2::Write(ConvertGroup23Var2::Apply(value), buff);
}

// ------- Group23Var3 -------

Group23Var3::Group23Var3() : flags(0), value(0) {}

bool Group23Var3::Read(rseq_t& buffer, Group23Var3& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group23Var3::Write(const Group23Var3& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group23Var3::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var3 value;
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

bool Group23Var3::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var3::Write(ConvertGroup23Var3::Apply(value), buff);
}

// ------- Group23Var4 -------

Group23Var4::Group23Var4() : flags(0), value(0) {}

bool Group23Var4::Read(rseq_t& buffer, Group23Var4& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group23Var4::Write(const Group23Var4& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group23Var4::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var4 value;
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

bool Group23Var4::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var4::Write(ConvertGroup23Var4::Apply(value), buff);
}

// ------- Group23Var5 -------

Group23Var5::Group23Var5() : flags(0), value(0), time(0) {}

bool Group23Var5::Read(rseq_t& buffer, Group23Var5& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group23Var5::Write(const Group23Var5& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group23Var5::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var5 value;
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

bool Group23Var5::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var5::Write(ConvertGroup23Var5::Apply(value), buff);
}

// ------- Group23Var6 -------

Group23Var6::Group23Var6() : flags(0), value(0), time(0) {}

bool Group23Var6::Read(rseq_t& buffer, Group23Var6& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group23Var6::Write(const Group23Var6& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group23Var6::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var6 value;
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

bool Group23Var6::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var6::Write(ConvertGroup23Var6::Apply(value), buff);
}

// ------- Group23Var7 -------

Group23Var7::Group23Var7() : flags(0), value(0), time(0) {}

bool Group23Var7::Read(rseq_t& buffer, Group23Var7& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group23Var7::Write(const Group23Var7& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group23Var7::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var7 value;
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

bool Group23Var7::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var7::Write(ConvertGroup23Var7::Apply(value), buff);
}

// ------- Group23Var8 -------

Group23Var8::Group23Var8() : flags(0), value(0), time(0) {}

bool Group23Var8::Read(rseq_t& buffer, Group23Var8& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group23Var8::Write(const Group23Var8& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group23Var8::ReadTarget(rseq_t& buff, FrozenCounter& output)
{
    Group23Var8 value;
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

bool Group23Var8::WriteTarget(const FrozenCounter& value, ser4cpp::wseq_t& buff)
{
    return Group23Var8::Write(ConvertGroup23Var8::Apply(value), buff);
}

} // namespace opendnp3
