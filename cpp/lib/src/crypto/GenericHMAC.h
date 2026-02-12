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
#ifndef OPENDNP3_GENERICHMAC_H
#define OPENDNP3_GENERICHMAC_H

#include <ser4cpp/container/SequenceTypes.h>

#include <openssl/evp.h>

#include <cstdint>
#include <initializer_list>
#include <system_error>

namespace opendnp3
{

/**
 * Generic HMAC calculation using OpenSSL EVP_MD.
 */
ser4cpp::rseq_t CalculateHMAC(const EVP_MD* md,
                              uint32_t outputSize,
                              const ser4cpp::rseq_t& key,
                              std::initializer_list<ser4cpp::rseq_t> data,
                              ser4cpp::wseq_t& output,
                              std::error_code& ec);

} // namespace opendnp3

#endif
