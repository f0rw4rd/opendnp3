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
#ifndef OPENDNP3_ABORTFILETASK_H
#define OPENDNP3_ABORTFILETASK_H

#include "master/IMasterTask.h"
#include "master/TaskPriority.h"

#include "opendnp3/master/FileOperationResult.h"

namespace opendnp3
{

/**
 * Master task for ABORT_FILE (FC=0x1E).
 * Sends Group70Var4 with the file handle, expects a response with Group70Var4.
 */
class AbortFileTask final : public IMasterTask
{
public:
    AbortFileTask(const std::shared_ptr<TaskContext>& context,
                  IMasterApplication& app,
                  uint32_t fileHandle,
                  FileOperationCallbackT callback,
                  const Logger& logger,
                  const TaskConfig& config);

    char const* Name() const override
    {
        return "Abort File";
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

    const uint32_t fileHandle;
    FileOperationCallbackT callback;
    FileStatus lastStatus;
};

} // namespace opendnp3

#endif
