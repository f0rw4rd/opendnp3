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

#include "Group33.h"

#include "app/MeasurementFactory.h"
#include "app/WriteConversions.h"
#include "app/parsing/DNPTimeParsing.h"

#include <ser4cpp/serialization/LittleEndian.h>

using namespace ser4cpp;

namespace opendnp3
{

// ------- Group33Var1 -------

Group33Var1::Group33Var1() : flags(0), value(0) {}

bool Group33Var1::Read(rseq_t& buffer, Group33Var1& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group33Var1::Write(const Group33Var1& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group33Var1::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var1 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var1::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var1::Write(ConvertGroup33Var1::Apply(value), buff);
}

// ------- Group33Var2 -------

Group33Var2::Group33Var2() : flags(0), value(0) {}

bool Group33Var2::Read(rseq_t& buffer, Group33Var2& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group33Var2::Write(const Group33Var2& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group33Var2::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var2 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var2::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var2::Write(ConvertGroup33Var2::Apply(value), buff);
}

// ------- Group33Var3 -------

Group33Var3::Group33Var3() : flags(0), value(0), time(0) {}

bool Group33Var3::Read(rseq_t& buffer, Group33Var3& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group33Var3::Write(const Group33Var3& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group33Var3::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var3 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var3::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var3::Write(ConvertGroup33Var3::Apply(value), buff);
}

// ------- Group33Var4 -------

Group33Var4::Group33Var4() : flags(0), value(0), time(0) {}

bool Group33Var4::Read(rseq_t& buffer, Group33Var4& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group33Var4::Write(const Group33Var4& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group33Var4::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var4 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var4::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var4::Write(ConvertGroup33Var4::Apply(value), buff);
}

// ------- Group33Var5 -------

Group33Var5::Group33Var5() : flags(0), value(0.0) {}

bool Group33Var5::Read(rseq_t& buffer, Group33Var5& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group33Var5::Write(const Group33Var5& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group33Var5::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var5 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var5::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var5::Write(ConvertGroup33Var5::Apply(value), buff);
}

// ------- Group33Var6 -------

Group33Var6::Group33Var6() : flags(0), value(0.0) {}

bool Group33Var6::Read(rseq_t& buffer, Group33Var6& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group33Var6::Write(const Group33Var6& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group33Var6::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var6 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var6::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var6::Write(ConvertGroup33Var6::Apply(value), buff);
}

// ------- Group33Var7 -------

Group33Var7::Group33Var7() : flags(0), value(0.0), time(0) {}

bool Group33Var7::Read(rseq_t& buffer, Group33Var7& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group33Var7::Write(const Group33Var7& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group33Var7::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var7 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var7::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var7::Write(ConvertGroup33Var7::Apply(value), buff);
}

// ------- Group33Var8 -------

Group33Var8::Group33Var8() : flags(0), value(0.0), time(0) {}

bool Group33Var8::Read(rseq_t& buffer, Group33Var8& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group33Var8::Write(const Group33Var8& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group33Var8::ReadTarget(rseq_t& buff, Analog& output)
{
    Group33Var8 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.flags, value.value, value.time);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group33Var8::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group33Var8::Write(ConvertGroup33Var8::Apply(value), buff);
}

} // namespace opendnp3
