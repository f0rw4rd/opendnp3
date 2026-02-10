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
#ifndef OPENDNP3_FREEZETYPE_H
#define OPENDNP3_FREEZETYPE_H

#include <cstdint>

namespace opendnp3
{

/**
 * Enumeration of freeze operation types for master freeze requests.
 */
enum class FreezeType : uint8_t
{
    /// Immediate freeze: copy current values to freeze buffer
    ImmediateFreeze,
    /// Immediate freeze, no response expected from outstation
    ImmediateFreezeNR,
    /// Freeze and clear: copy current values to freeze buffer, then clear
    FreezeAndClear,
    /// Freeze and clear, no response expected from outstation
    FreezeAndClearNR
};

} // namespace opendnp3

#endif
