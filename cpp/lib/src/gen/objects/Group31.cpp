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

#include "Group31.h"

#include "app/MeasurementFactory.h"
#include "app/WriteConversions.h"
#include "app/parsing/DNPTimeParsing.h"

#include <ser4cpp/serialization/LittleEndian.h>

using namespace ser4cpp;

namespace opendnp3
{

// ------- Group31Var1 -------

Group31Var1::Group31Var1() : flags(0), value(0) {}

bool Group31Var1::Read(rseq_t& buffer, Group31Var1& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group31Var1::Write(const Group31Var1& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group31Var1::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var1 value;
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

bool Group31Var1::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var1::Write(ConvertGroup31Var1::Apply(value), buff);
}

// ------- Group31Var2 -------

Group31Var2::Group31Var2() : flags(0), value(0) {}

bool Group31Var2::Read(rseq_t& buffer, Group31Var2& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group31Var2::Write(const Group31Var2& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group31Var2::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var2 value;
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

bool Group31Var2::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var2::Write(ConvertGroup31Var2::Apply(value), buff);
}

// ------- Group31Var3 -------

Group31Var3::Group31Var3() : flags(0), value(0), time(0) {}

bool Group31Var3::Read(rseq_t& buffer, Group31Var3& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group31Var3::Write(const Group31Var3& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group31Var3::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var3 value;
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

bool Group31Var3::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var3::Write(ConvertGroup31Var3::Apply(value), buff);
}

// ------- Group31Var4 -------

Group31Var4::Group31Var4() : flags(0), value(0), time(0) {}

bool Group31Var4::Read(rseq_t& buffer, Group31Var4& output)
{
    return LittleEndian::read(buffer, output.flags, output.value, output.time);
}

bool Group31Var4::Write(const Group31Var4& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value, arg.time);
}

bool Group31Var4::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var4 value;
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

bool Group31Var4::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var4::Write(ConvertGroup31Var4::Apply(value), buff);
}

// ------- Group31Var5 -------

Group31Var5::Group31Var5() : value(0) {}

bool Group31Var5::Read(rseq_t& buffer, Group31Var5& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group31Var5::Write(const Group31Var5& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group31Var5::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var5 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group31Var5::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var5::Write(ConvertGroup31Var5::Apply(value), buff);
}

// ------- Group31Var6 -------

Group31Var6::Group31Var6() : value(0) {}

bool Group31Var6::Read(rseq_t& buffer, Group31Var6& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group31Var6::Write(const Group31Var6& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group31Var6::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var6 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(value.value);
        return true;
    }
    else
    {
        return false;
    }
}

bool Group31Var6::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var6::Write(ConvertGroup31Var6::Apply(value), buff);
}

// ------- Group31Var7 -------

Group31Var7::Group31Var7() : flags(0), value(0.0) {}

bool Group31Var7::Read(rseq_t& buffer, Group31Var7& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group31Var7::Write(const Group31Var7& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group31Var7::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var7 value;
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

bool Group31Var7::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var7::Write(ConvertGroup31Var7::Apply(value), buff);
}

// ------- Group31Var8 -------

Group31Var8::Group31Var8() : flags(0), value(0.0) {}

bool Group31Var8::Read(rseq_t& buffer, Group31Var8& output)
{
    return LittleEndian::read(buffer, output.flags, output.value);
}

bool Group31Var8::Write(const Group31Var8& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.flags, arg.value);
}

bool Group31Var8::ReadTarget(rseq_t& buff, Analog& output)
{
    Group31Var8 value;
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

bool Group31Var8::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    return Group31Var8::Write(ConvertGroup31Var8::Apply(value), buff);
}

} // namespace opendnp3
