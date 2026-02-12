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
#ifndef OPENDNP3_SECURECOMPARE_H
#define OPENDNP3_SECURECOMPARE_H

#include <ser4cpp/container/SequenceTypes.h>

namespace opendnp3
{

/**
 * Constant-time buffer comparison that does not leak timing information.
 * Used to compare HMAC values and other security-sensitive data.
 *
 * @param lhs First buffer
 * @param rhs Second buffer
 * @return true if buffers are equal in length and content
 */
bool SecureEquals(const ser4cpp::rseq_t& lhs, const ser4cpp::rseq_t& rhs);

} // namespace opendnp3

#endif
