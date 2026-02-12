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
#include "secauth/outstation/SessionKeyUnwrapBuffer.h"

#include "logging/LogMacros.h"

#include "opendnp3/logging/LogLevels.h"

#include <ser4cpp/serialization/LittleEndian.h>

namespace opendnp3
{

SessionKeyUnwrapBuffer::Result::Result() : success(false) {}

SessionKeyUnwrapBuffer::Result::Result(const SessionKeysView& keys_, const ser4cpp::rseq_t& keyStatusObject_)
    : success(true), keys(keys_), keyStatusObject(keyStatusObject_)
{
}

SessionKeyUnwrapBuffer::Result SessionKeyUnwrapBuffer::Unwrap(IKeyWrapAlgo& algo,
                                                              const ser4cpp::rseq_t& updateKey,
                                                              const ser4cpp::rseq_t& inputData,
                                                              Logger* pLogger)
{
    ser4cpp::wseq_t dest(buffer.data(), buffer.size());

    std::error_code ec;
    auto unwrapped = algo.UnwrapKey(updateKey, inputData, dest, ec);

    if (ec)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, ec.message().c_str());
        return Result::Failure();
    }

    if (unwrapped.length() < 2)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for key length");
        return Result::Failure();
    }

    uint16_t keyLength = 0;
    ser4cpp::UInt16::read_from(unwrapped, keyLength);

    if (!AuthSizes::SessionKeySizeWithinLimits(keyLength))
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Session key size not within limits");
        return Result::Failure();
    }

    const uint32_t REQUIRED_KEY_SIZE = 2 * keyLength;

    if (unwrapped.length() < REQUIRED_KEY_SIZE)
    {
        SIMPLE_LOGGER_BLOCK(pLogger, flags::WARN, "Not enough data for session keys");
        return Result::Failure();
    }

    auto controlKey = unwrapped.take(keyLength);
    unwrapped.advance(keyLength);

    auto monitorKey = unwrapped.take(keyLength);
    unwrapped.advance(keyLength);

    // Anything left over is the key status message
    return Result(SessionKeysView(controlKey, monitorKey), unwrapped);
}

} // namespace opendnp3
