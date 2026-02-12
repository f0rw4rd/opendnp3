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
#include "secauth/outstation/AuthorityCredentials.h"

namespace opendnp3
{

AuthorityCredentials::AuthorityCredentials() : m_statusChangeSeqNum(0) {}

void AuthorityCredentials::SetSCSN(uint32_t statusChangeSeqNumber)
{
    m_statusChangeSeqNum = statusChangeSeqNumber;
}

void AuthorityCredentials::Configure(uint32_t statusChangeSeqNumber, const AuthorityKey& key)
{
    m_statusChangeSeqNum = statusChangeSeqNumber;
    m_authorityKey = AuthorityKey(key.GetView());
}

bool AuthorityCredentials::GetSymmetricKey(uint32_t& statusChangeSeqNumber, ser4cpp::rseq_t& keyView) const
{
    if (!m_authorityKey.IsValid())
    {
        return false;
    }
    statusChangeSeqNumber = m_statusChangeSeqNum;
    keyView = m_authorityKey.GetView();
    return true;
}

ser4cpp::rseq_t AuthorityCredentials::GetSymmetricKey() const
{
    return m_authorityKey.GetView();
}

} // namespace opendnp3
