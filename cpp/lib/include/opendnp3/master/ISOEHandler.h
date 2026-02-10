/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Modified 2024-2026 f0rw4rd (experimental fork)
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
#ifndef OPENDNP3_ISOEHANDLER_H
#define OPENDNP3_ISOEHANDLER_H

#include "opendnp3/app/AnalogCommandEvent.h"
#include "opendnp3/app/BinaryCommandEvent.h"
#include "opendnp3/app/DeviceAttributes.h"
#include "opendnp3/app/Indexed.h"
#include "opendnp3/app/MeasurementTypes.h"
#include "opendnp3/app/OctetString.h"
#include "opendnp3/app/parsing/ICollection.h"
#include "opendnp3/master/HeaderInfo.h"
#include "opendnp3/master/ResponseInfo.h"

#include <cstddef>

namespace opendnp3
{

/**
 * An interface for Sequence-Of-Events (SOE) callbacks from a master stack to
 * the application layer.
 *
 * A call is made to the appropriate member method for every measurement value in an ASDU.
 * The HeaderInfo class provides information about the object header associated with the value.
 *
 */
class ISOEHandler
{

public:
    virtual ~ISOEHandler() = default;

    virtual void BeginFragment(const ResponseInfo& info) = 0;
    virtual void EndFragment(const ResponseInfo& info) = 0;

    virtual void OnRawAPDU(const ResponseInfo& info, const uint8_t* data, size_t length) {}

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<DoubleBitBinary>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Counter>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<FrozenCounter>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<OctetString>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<TimeAndInterval>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryCommandEvent>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogCommandEvent>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogInputDeadband>>& values) = 0;
    virtual void Process(const HeaderInfo& info, const ICollection<DNPTime>& values) = 0;

    /**
     * Called when a device attribute (Group 0) is received.
     *
     * @param info Header information
     * @param set The attribute set (index from the range qualifier)
     * @param variation The attribute variation number
     * @param value Parsed attribute value
     *
     * Default implementation does nothing, so existing code is unaffected.
     */
    virtual void OnDeviceAttribute(const HeaderInfo& info,
                                   uint8_t set,
                                   uint8_t variation,
                                   const DeviceAttributeValue& value)
    {
    }
};

} // namespace opendnp3

#endif
