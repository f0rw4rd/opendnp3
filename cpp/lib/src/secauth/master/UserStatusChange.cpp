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
#include "secauth/master/UserStatusChange.h"

#include "gen/objects/Group120.h"
#include "secauth/StringConversions.h"

namespace opendnp3
{

UserStatusChange::UserStatusChange(KeyChangeMethod keyChangeMethod_,
                                   UserOperation userOperation_,
                                   uint32_t statusChangeSeqNum_,
                                   uint16_t userRole_,
                                   uint16_t userRoleExpDays_,
                                   const std::string& userName_,
                                   const ser4cpp::rseq_t& userPublicKey_,
                                   const ser4cpp::rseq_t& certificationData_)
    : keyChangeMethod(keyChangeMethod_),
      userOperation(userOperation_),
      statusChangeSeqNum(statusChangeSeqNum_),
      userRole(userRole_),
      userRoleExpDays(userRoleExpDays_),
      userName(userName_),
      userPublicKey(static_cast<const uint8_t*>(userPublicKey_),
                    static_cast<const uint8_t*>(userPublicKey_) + userPublicKey_.length()),
      certificationData(static_cast<const uint8_t*>(certificationData_),
                        static_cast<const uint8_t*>(certificationData_) + certificationData_.length())
{
}

Group120Var10 UserStatusChange::Convert() const
{
    ser4cpp::rseq_t pubKeyView(userPublicKey.data(), userPublicKey.size());
    ser4cpp::rseq_t certView(certificationData.data(), certificationData.size());

    return Group120Var10(keyChangeMethod, userOperation, statusChangeSeqNum, userRole, userRoleExpDays,
                         AsSlice(userName), pubKeyView, certView);
}

} // namespace opendnp3
