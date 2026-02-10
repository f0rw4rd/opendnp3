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

#include "Group34.h"

#include <ser4cpp/serialization/LittleEndian.h>

using namespace ser4cpp;

namespace opendnp3
{

// ------- Group34Var1 -------

Group34Var1::Group34Var1() : value(0) {}

bool Group34Var1::Read(rseq_t& buffer, Group34Var1& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group34Var1::Write(const Group34Var1& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group34Var1::ReadTarget(rseq_t& buffer, AnalogInputDeadband& output)
{
    Group34Var1 raw;
    if (!Read(buffer, raw))
    {
        return false;
    }
    output = AnalogInputDeadband(static_cast<double>(raw.value));
    return true;
}

// ------- Group34Var2 -------

Group34Var2::Group34Var2() : value(0) {}

bool Group34Var2::Read(rseq_t& buffer, Group34Var2& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group34Var2::Write(const Group34Var2& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group34Var2::ReadTarget(rseq_t& buffer, AnalogInputDeadband& output)
{
    Group34Var2 raw;
    if (!Read(buffer, raw))
    {
        return false;
    }
    output = AnalogInputDeadband(static_cast<double>(raw.value));
    return true;
}

// ------- Group34Var3 -------

Group34Var3::Group34Var3() : value(0.0) {}

bool Group34Var3::Read(rseq_t& buffer, Group34Var3& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group34Var3::Write(const Group34Var3& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group34Var3::ReadTarget(rseq_t& buffer, AnalogInputDeadband& output)
{
    Group34Var3 raw;
    if (!Read(buffer, raw))
    {
        return false;
    }
    output = AnalogInputDeadband(static_cast<double>(raw.value));
    return true;
}

} // namespace opendnp3
