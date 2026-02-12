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
#ifndef OPENDNP3_BEGINUPDATEKEYCHANGERESULT_H
#define OPENDNP3_BEGINUPDATEKEYCHANGERESULT_H

#include "opendnp3/gen/TaskCompletion.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>
#include <vector>

namespace opendnp3
{

/**
 * Result of the first step in an update key change sequence.
 * Contains the outstation's assigned user number, KSQ, and challenge data from both sides.
 */
class BeginUpdateKeyChangeResult
{
public:
    /// Failure constructor
    explicit BeginUpdateKeyChangeResult(TaskCompletion result);

    /// Success constructor
    BeginUpdateKeyChangeResult(uint16_t userNum,
                               uint32_t keyChangeSequenceNum,
                               const ser4cpp::rseq_t& masterChallengeData,
                               const ser4cpp::rseq_t& outstationChallengeData);

    /// The success or failure type of the overall operation.
    /// The other fields are only valid if this value is SUCCESS.
    TaskCompletion result;

    /// The user number assigned by the outstation
    uint16_t userNum;

    /// The KSQ specified by the outstation
    uint32_t keyChangeSequenceNum;

    /// The challenge data that the master chose when it initiated the request
    std::vector<uint8_t> masterChallengeData;

    /// The challenge data that the outstation provided in its response
    std::vector<uint8_t> outstationChallengeData;
};

} // namespace opendnp3

#endif
