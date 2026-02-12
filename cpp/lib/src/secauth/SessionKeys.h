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
#ifndef OPENDNP3_SESSIONKEYS_H
#define OPENDNP3_SESSIONKEYS_H

#include "secauth/AuthSizes.h"
#include "secauth/SessionKeysView.h"

#include "opendnp3/crypto/ISecureRandom.h"
#include "opendnp3/util/Uncopyable.h"

#include <array>

namespace opendnp3
{

/// Bounded session key size
class SessionKeySize
{
public:
    explicit SessionKeySize(uint32_t size_) : size(AuthSizes::GetBoundedSessionKeySize(size_)) {}

    operator uint32_t() const
    {
        return size;
    }

private:
    uint32_t size;
};

/**
 * Stores bidirectional session key data (CDSK and MDSK) in internal buffers.
 */
class SessionKeys : private Uncopyable
{
public:
    /// Set keys from a view, copying the data into internal buffers
    void SetKeys(const SessionKeysView& view);

    /// Get a read-only view of the current keys
    SessionKeysView GetView() const;

    /// Generate new random session keys using a secure random source
    void DeriveFrom(ISecureRandom& rs, const SessionKeySize& size, std::error_code& ec);

private:
    uint32_t controlSize = 0;
    uint32_t monitorSize = 0;
    std::array<uint8_t, AuthSizes::MAX_SESSION_KEY_SIZE_BYTES> controlBuffer;
    std::array<uint8_t, AuthSizes::MAX_SESSION_KEY_SIZE_BYTES> monitorBuffer;
};

} // namespace opendnp3

#endif
