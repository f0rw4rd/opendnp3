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
#ifndef OPENDNP3_AESKEYWRAP_H
#define OPENDNP3_AESKEYWRAP_H

#include "opendnp3/crypto/IKeyWrapAlgo.h"
#include "opendnp3/util/Uncopyable.h"

namespace opendnp3
{

class AESKeyWrap final : public IKeyWrapAlgo
{
public:
    virtual ser4cpp::rseq_t WrapKey(const ser4cpp::rseq_t& kek,
                                    const ser4cpp::rseq_t& input,
                                    ser4cpp::wseq_t& output,
                                    std::error_code& ec) const override;

    virtual ser4cpp::rseq_t UnwrapKey(const ser4cpp::rseq_t& kek,
                                      const ser4cpp::rseq_t& input,
                                      ser4cpp::wseq_t& output,
                                      std::error_code& ec) const override;
};

} // namespace opendnp3

#endif
