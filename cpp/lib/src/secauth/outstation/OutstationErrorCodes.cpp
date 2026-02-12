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
#include "secauth/outstation/OutstationErrorCodes.h"

namespace opendnp3
{

OutstationErrorCategory OutstationErrorCategory::instance;

std::string OutstationErrorCategory::message(int ev) const
{
    switch (ev)
    {
    case (static_cast<int>(OutstationError::BAD_UNWRAPPPED_UPDATE_KEY_DATA_SIZE)):
        return "Unwrapped update key data doesn't meet size requirements";
    case (static_cast<int>(OutstationError::DECRYPTED_USERNAME_MISMATCH)):
        return "The decrypted username did not match the expected username";
    case (static_cast<int>(OutstationError::CHALLENGE_DATA_MISMATCH)):
        return "Decrypted challenge data did not match what was sent";
    case (static_cast<int>(OutstationError::KEY_CHANGE_CONFIRMATION_HMAC_MISMATCH)):
        return "The confirmation HMAC on a g120v15 did not match the expected value";
    default:
        return "unknown outstation error";
    }
}

std::error_code make_error_code(OutstationError err)
{
    return std::error_code(static_cast<int>(err), OutstationErrorCategory::Instance());
}

} // namespace opendnp3
