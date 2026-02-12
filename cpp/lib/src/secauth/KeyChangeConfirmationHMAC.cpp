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
#include "secauth/KeyChangeConfirmationHMAC.h"

#include <ser4cpp/serialization/LittleEndian.h>

namespace opendnp3
{

ser4cpp::rseq_t KeyChangeConfirmationHMAC::Compute(IHMACAlgo& algo,
                                                   const ser4cpp::rseq_t& key,
                                                   uint32_t keyChangeSeqNum,
                                                   uint16_t userNum,
                                                   const ser4cpp::rseq_t& outstationChallengeData,
                                                   const ser4cpp::rseq_t& masterChallengeData,
                                                   std::error_code& ec)
{
    // Serialize the fixed fields: KSQ (4 bytes) + user number (2 bytes)
    uint8_t fixedBuf[6];
    ser4cpp::wseq_t fixedWriter(fixedBuf, 6);
    ser4cpp::UInt32::write_to(fixedWriter, keyChangeSeqNum);
    ser4cpp::UInt16::write_to(fixedWriter, userNum);

    ser4cpp::rseq_t fixedData(fixedBuf, 6);

    ser4cpp::wseq_t dest(m_buffer.data(), static_cast<uint32_t>(m_buffer.size()));

    return algo.Calculate(key, {fixedData, outstationChallengeData, masterChallengeData}, dest, ec);
}

} // namespace opendnp3
