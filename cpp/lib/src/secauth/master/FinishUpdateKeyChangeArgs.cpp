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
#include "secauth/master/FinishUpdateKeyChangeArgs.h"

namespace opendnp3
{

FinishUpdateKeyChangeArgs::FinishUpdateKeyChangeArgs(const std::string& username_,
                                                     const std::string& outstationName_,
                                                     uint16_t userNum_,
                                                     uint32_t keyChangeSequenceNumber_,
                                                     const ser4cpp::rseq_t& masterChallengeData_,
                                                     const ser4cpp::rseq_t& outstationChallengeData_,
                                                     const ser4cpp::rseq_t& encryptedKeyData_,
                                                     const UpdateKey& updateKey_)
    : username(username_),
      outstationName(outstationName_),
      userNum(userNum_),
      keyChangeSequenceNum(keyChangeSequenceNumber_),
      masterChallengeData(static_cast<const uint8_t*>(masterChallengeData_),
                          static_cast<const uint8_t*>(masterChallengeData_) + masterChallengeData_.length()),
      outstationChallengeData(static_cast<const uint8_t*>(outstationChallengeData_),
                              static_cast<const uint8_t*>(outstationChallengeData_)
                                  + outstationChallengeData_.length()),
      encryptedKeyData(static_cast<const uint8_t*>(encryptedKeyData_),
                       static_cast<const uint8_t*>(encryptedKeyData_) + encryptedKeyData_.length()),
      updateKey(updateKey_)
{
}

} // namespace opendnp3
