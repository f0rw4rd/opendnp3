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
#include "secauth/StringConversions.h"

namespace opendnp3
{

std::string ToString(const ser4cpp::rseq_t& data)
{
    std::string str;
    const uint8_t* buffer = data;
    str.append(reinterpret_cast<const char*>(buffer), data.length());
    return str;
}

ser4cpp::rseq_t AsSlice(const std::string& str)
{
    return ser4cpp::rseq_t(reinterpret_cast<const uint8_t*>(str.c_str()), static_cast<uint32_t>(str.size()));
}

} // namespace opendnp3
