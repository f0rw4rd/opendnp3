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
#ifndef OPENDNP3_STRINGCONVERSIONS_H
#define OPENDNP3_STRINGCONVERSIONS_H

#include <ser4cpp/container/SequenceTypes.h>

#include <string>

namespace opendnp3
{

/// Makes a copy of the data and stores it in std::string
std::string ToString(const ser4cpp::rseq_t& data);

/// Produces an unsigned byte view of the string without copying
ser4cpp::rseq_t AsSlice(const std::string& str);

} // namespace opendnp3

#endif
