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

#include "Group102.h"

#include "app/MeasurementFactory.h"

#include <ser4cpp/serialization/LittleEndian.h>

using namespace ser4cpp;

namespace opendnp3
{

// ------- Group102Var1 -------

Group102Var1::Group102Var1() : value(0) {}

bool Group102Var1::Read(rseq_t& buffer, Group102Var1& output)
{
    return LittleEndian::read(buffer, output.value);
}

bool Group102Var1::Write(const Group102Var1& arg, ser4cpp::wseq_t& buffer)
{
    return LittleEndian::write(buffer, arg.value);
}

bool Group102Var1::ReadTarget(rseq_t& buff, Analog& output)
{
    Group102Var1 value;
    if (Read(buff, value))
    {
        output = AnalogFactory::From(static_cast<int32_t>(value.value));
        return true;
    }
    else
    {
        return false;
    }
}

bool Group102Var1::WriteTarget(const Analog& value, ser4cpp::wseq_t& buff)
{
    Group102Var1 obj;
    obj.value = static_cast<uint8_t>(value.value);
    return Group102Var1::Write(obj, buff);
}

} // namespace opendnp3
