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
#ifndef OPENDNP3_SHA1HMAC_H
#define OPENDNP3_SHA1HMAC_H

#include "opendnp3/crypto/IHMACAlgo.h"
#include "opendnp3/util/Uncopyable.h"

namespace opendnp3
{

class SHA1HMAC final : public IHMACAlgo, private Uncopyable
{
public:
    virtual uint16_t OutputSize() const override
    {
        return OUTPUT_SIZE;
    }

    virtual ser4cpp::rseq_t Calculate(const ser4cpp::rseq_t& key,
                                      std::initializer_list<ser4cpp::rseq_t> data,
                                      ser4cpp::wseq_t& dest,
                                      std::error_code& ec) override;

private:
    static const uint16_t OUTPUT_SIZE = 20;
};

} // namespace opendnp3

#endif
