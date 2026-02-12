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
#ifndef OPENDNP3_OUTSTATIONERRORCODES_H
#define OPENDNP3_OUTSTATIONERRORCODES_H

#include <string>
#include <system_error>

namespace opendnp3
{

enum class OutstationError : int
{
    BAD_UNWRAPPPED_UPDATE_KEY_DATA_SIZE,
    DECRYPTED_USERNAME_MISMATCH,
    CHALLENGE_DATA_MISMATCH,
    KEY_CHANGE_CONFIRMATION_HMAC_MISMATCH
};

class OutstationErrorCategory final : public std::error_category
{
public:
    static const std::error_category& Instance()
    {
        return instance;
    }

    virtual const char* name() const noexcept override
    {
        return "Outstation Errors";
    }

    virtual std::string message(int ev) const override;

private:
    OutstationErrorCategory() {}
    OutstationErrorCategory(const OutstationErrorCategory&) = delete;

    static OutstationErrorCategory instance;
};

std::error_code make_error_code(OutstationError err);

} // namespace opendnp3

namespace std
{
template<> struct is_error_code_enum<opendnp3::OutstationError> : public true_type
{
};
} // namespace std

#endif
