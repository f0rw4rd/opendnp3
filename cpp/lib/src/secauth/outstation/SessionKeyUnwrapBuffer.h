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
#ifndef OPENDNP3_SESSIONKEYUNWRAPBUFFER_H
#define OPENDNP3_SESSIONKEYUNWRAPBUFFER_H

#include "secauth/AuthSizes.h"
#include "secauth/SessionKeysView.h"

#include "opendnp3/crypto/IKeyWrapAlgo.h"
#include "opendnp3/logging/Logger.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <array>

namespace opendnp3
{

/**
 * Buffer for unwrapping session keys received from a master.
 * The wrapped data format is: keyLength(2) + controlKey(N) + monitorKey(N) + keyStatusMessage(...)
 */
class SessionKeyUnwrapBuffer
{
public:
    class Result
    {
    public:
        static Result Failure()
        {
            return Result();
        }

        Result(const SessionKeysView& keys, const ser4cpp::rseq_t& keyStatusObject);

        bool success;
        SessionKeysView keys;
        ser4cpp::rseq_t keyStatusObject;

    private:
        Result();
    };

    Result Unwrap(IKeyWrapAlgo& algo,
                  const ser4cpp::rseq_t& updateKey,
                  const ser4cpp::rseq_t& inputData,
                  Logger* pLogger);

private:
    std::array<uint8_t, AuthSizes::MAX_SESSION_KEY_WRAP_BUFFER_SIZE> buffer;
};

} // namespace opendnp3

#endif
