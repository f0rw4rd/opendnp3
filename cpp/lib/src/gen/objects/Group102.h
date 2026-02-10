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

#ifndef OPENDNP3_GROUP102_H
#define OPENDNP3_GROUP102_H

#include "app/DNP3Serializer.h"
#include "app/MeasurementTypeSpecs.h"

#include "opendnp3/app/GroupVariationID.h"

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

// Unsigned Integer Without Flag - Any Variation
struct Group102Var0
{
    static GroupVariationID ID()
    {
        return GroupVariationID(102, 0);
    }
};

// Unsigned Integer Without Flag - 8-bit
struct Group102Var1
{
    static GroupVariationID ID()
    {
        return GroupVariationID(102, 1);
    }

    Group102Var1();

    static size_t Size()
    {
        return 1;
    }
    static bool Read(ser4cpp::rseq_t&, Group102Var1&);
    static bool Write(const Group102Var1&, ser4cpp::wseq_t&);

    typedef uint8_t ValueType;
    uint8_t value;

    typedef Analog Target;
    typedef AnalogSpec Spec;
    static bool ReadTarget(ser4cpp::rseq_t&, Analog&);
    static bool WriteTarget(const Analog&, ser4cpp::wseq_t&);
    static DNP3Serializer<Analog> Inst()
    {
        return DNP3Serializer<Analog>(ID(), Size(), &ReadTarget, &WriteTarget);
    }
};

} // namespace opendnp3

#endif
