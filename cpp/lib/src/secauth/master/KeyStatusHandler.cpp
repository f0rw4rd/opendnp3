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
#include "secauth/master/KeyStatusHandler.h"

namespace opendnp3
{

KeyStatusHandler::KeyStatusHandler(Logger logger_) : logger(logger_), valid(false) {}

bool KeyStatusHandler::GetStatus(Group120Var5& status_, ser4cpp::rseq_t& rawObject_) const
{
    if (valid)
    {
        status_ = this->status;
        rawObject_ = this->rawObject;
    }

    return valid;
}

IINField KeyStatusHandler::ProcessHeader(const FreeFormatHeader& header,
                                         const Group120Var5& status_,
                                         const ser4cpp::rseq_t& rawObject_)
{
    this->valid = true;
    this->status = status_;
    this->rawObject = rawObject_;
    return IINField::Empty();
}

} // namespace opendnp3
