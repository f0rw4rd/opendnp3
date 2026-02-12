/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OPENDNP3_PERMISSIONS_H
#define OPENDNP3_PERMISSIONS_H

#include "opendnp3/gen/FunctionCode.h"

#include <cstdint>

namespace opendnp3
{

/**
 * Describes what function codes a user is authorized to perform.
 * Uses a 64-bit bitfield where each bit corresponds to a function code.
 */
class Permissions
{
    typedef uint64_t bitfield_t;

public:
    Permissions() : permissions(0) {}

    /// Variadic factory for creating permissions from a list of function codes
    template<typename... Args> static Permissions Allowed(Args... args)
    {
        return Permissions(GetBitfield(args...));
    }

    Permissions operator|(const Permissions& other) const
    {
        return Permissions(this->permissions | other.permissions);
    }

    /// No permissions
    static Permissions AllowNothing();

    /// All permissions
    static Permissions AllowAll();

    /// Add a function code to the allowed set
    void Allow(FunctionCode code);

    /// Check if a function code is allowed
    bool IsAllowed(FunctionCode code) const;

private:
    template<typename... Args> static bitfield_t GetBitfield(FunctionCode fc, Args... args)
    {
        return GetMask(fc) | GetBitfield(args...);
    }

    static bitfield_t GetBitfield()
    {
        return 0;
    }

    inline static bitfield_t Bit(uint8_t bit)
    {
        return static_cast<bitfield_t>(static_cast<uint64_t>(1) << bit);
    }

    static bitfield_t GetMask(FunctionCode code);

    bitfield_t permissions;

    explicit Permissions(bitfield_t mask);
};

} // namespace opendnp3

#endif
