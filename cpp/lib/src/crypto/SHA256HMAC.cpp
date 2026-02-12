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
#include "crypto/SHA256HMAC.h"

#include "crypto/GenericHMAC.h"

#include <openssl/evp.h>

namespace opendnp3
{

ser4cpp::rseq_t SHA256HMAC::Calculate(const ser4cpp::rseq_t& key,
                                      std::initializer_list<ser4cpp::rseq_t> data,
                                      ser4cpp::wseq_t& output,
                                      std::error_code& ec)
{
    return CalculateHMAC(EVP_sha256(), OUTPUT_SIZE, key, data, output, ec);
}

} // namespace opendnp3
