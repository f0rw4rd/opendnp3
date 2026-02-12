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
#include "secauth/SessionKeys.h"

#include <algorithm>
#include <cstring>

namespace opendnp3
{

void SessionKeys::SetKeys(const SessionKeysView& view)
{
    controlSize = static_cast<uint32_t>(
        std::min(view.controlKey.length(), static_cast<size_t>(AuthSizes::MAX_SESSION_KEY_SIZE_BYTES)));
    monitorSize = static_cast<uint32_t>(
        std::min(view.monitorKey.length(), static_cast<size_t>(AuthSizes::MAX_SESSION_KEY_SIZE_BYTES)));

    std::memcpy(controlBuffer.data(), view.controlKey, controlSize);
    std::memcpy(monitorBuffer.data(), view.monitorKey, monitorSize);
}

SessionKeysView SessionKeys::GetView() const
{
    return SessionKeysView(ser4cpp::rseq_t(controlBuffer.data(), controlSize),
                           ser4cpp::rseq_t(monitorBuffer.data(), monitorSize));
}

void SessionKeys::DeriveFrom(ISecureRandom& rs, const SessionKeySize& size, std::error_code& ec)
{
    controlSize = static_cast<uint32_t>(size);
    monitorSize = static_cast<uint32_t>(size);

    ser4cpp::wseq_t controlDest(controlBuffer.data(), controlSize);
    rs.GetSecureRandom(controlDest, ec);
    if (ec)
        return;

    ser4cpp::wseq_t monitorDest(monitorBuffer.data(), monitorSize);
    rs.GetSecureRandom(monitorDest, ec);
}

} // namespace opendnp3
