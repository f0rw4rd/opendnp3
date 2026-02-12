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
#ifndef OPENDNP3_AUTHORITYCREDENTIALS_H
#define OPENDNP3_AUTHORITYCREDENTIALS_H

#include "secauth/AuthorityKey.h"

#include <ser4cpp/container/SequenceTypes.h>

#include <cstdint>

namespace opendnp3
{

/**
 * Stores the authority's symmetric key and status change sequence number.
 * Used by the outstation to verify user status change messages signed by the authority.
 */
class AuthorityCredentials
{
public:
    AuthorityCredentials();

    void SetSCSN(uint32_t statusChangeSeqNumber);

    void Configure(uint32_t statusChangeSeqNumber, const AuthorityKey& key);

    bool GetSymmetricKey(uint32_t& statusChangeSeqNumber, ser4cpp::rseq_t& keyView) const;

    ser4cpp::rseq_t GetSymmetricKey() const;

private:
    uint32_t m_statusChangeSeqNum;
    AuthorityKey m_authorityKey;
};

} // namespace opendnp3

#endif
