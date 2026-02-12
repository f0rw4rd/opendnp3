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
#ifndef OPENDNP3_SINGLEOBJECTHANDLERS_H
#define OPENDNP3_SINGLEOBJECTHANDLERS_H

#include "gen/objects/Group120.h"
#include "secauth/SingleValueHandler.h"

namespace opendnp3
{

class ChallengeHandler
    : public SingleValueHandler<Group120Var1, GroupVariation::Group120Var1, QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var1& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class ChallengeReplyHandler
    : public SingleValueHandler<Group120Var2, GroupVariation::Group120Var2, QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var2& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class ChangeSessionKeysHandler
    : public SingleValueHandler<Group120Var6, GroupVariation::Group120Var6, QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var6& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class UserStatusChangeHandler : public SingleValueHandler<Group120Var10,
                                                          GroupVariation::Group120Var10,
                                                          QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var10& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class BeginUpdateKeyChangeRequestHandler : public SingleValueHandler<Group120Var11,
                                                                     GroupVariation::Group120Var11,
                                                                     QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var11& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class UpdateKeyChangeReplyHandler : public SingleValueHandler<Group120Var12,
                                                              GroupVariation::Group120Var12,
                                                              QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var12& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class ErrorHandler
    : public SingleValueHandler<Group120Var7, GroupVariation::Group120Var7, QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var7& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

class KeyChangeConfirmationHandler : public SingleValueHandler<Group120Var15,
                                                               GroupVariation::Group120Var15,
                                                               QualifierCode::UINT8_CNT_UINT16_FREE_FORMAT>
{
public:
    virtual IINField ProcessHeader(const FreeFormatHeader& header,
                                   const Group120Var15& data,
                                   const ser4cpp::rseq_t&) override final
    {
        this->value = data;
        this->m_valid = true;
        return IINField();
    }
};

} // namespace opendnp3

#endif
