/*
 * Copyright 2013-2022 Step Function I/O, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Step Function I/O
 * LLC (https://stepfunc.io) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Step Function I/O LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OPENDNP3_GETFILEINFOTASK_H
#define OPENDNP3_GETFILEINFOTASK_H

#include "master/IMasterTask.h"
#include "master/TaskPriority.h"

#include "opendnp3/master/FileOperationResult.h"

#include <string>

namespace opendnp3
{

/**
 * Master task for GET_FILE_INFO (FC=0x1C).
 * Sends Group70Var8 (filename), expects Group70Var7 (file descriptor) in response.
 */
class GetFileInfoTask final : public IMasterTask
{
public:
    GetFileInfoTask(const std::shared_ptr<TaskContext>& context,
                    IMasterApplication& app,
                    const std::string& filename,
                    FileInfoCallbackT callback,
                    const Logger& logger,
                    const TaskConfig& config);

    char const* Name() const override
    {
        return "Get File Info";
    }
    int Priority() const override
    {
        return priority::COMMAND;
    }
    bool BlocksLowerPriority() const override
    {
        return false;
    }
    bool IsRecurring() const override
    {
        return false;
    }
    bool BuildRequest(APDURequest& request, uint8_t seq) override;

private:
    MasterTaskType GetTaskType() const override
    {
        return MasterTaskType::USER_TASK;
    }
    ResponseResult ProcessResponse(const APDUResponseHeader& header, const ser4cpp::rseq_t& objects) override;
    void OnTaskComplete(TaskCompletion result, Timestamp now) override;

    bool ParseGroup70Var7(const ser4cpp::rseq_t& objects);

    const std::string filename;
    FileInfoCallbackT callback;
    FileInfo result;
    FileStatus lastStatus;
};

} // namespace opendnp3

#endif
