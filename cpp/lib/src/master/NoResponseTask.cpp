/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Created 2024-2026 f0rw4rd (experimental fork)
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
#include "master/NoResponseTask.h"

#include "master/TaskPriority.h"

#include <utility>

namespace opendnp3
{

NoResponseTask::NoResponseTask(const std::shared_ptr<TaskContext>& context,
                               IMasterApplication& app,
                               std::string name,
                               FunctionCode func,
                               HeaderBuilderT format,
                               Timestamp startExpiration,
                               const Logger& logger,
                               const TaskConfig& config)
    : IMasterTask(context, app, TaskBehavior::SingleExecutionNoRetry(startExpiration), logger, config),
      name(std::move(name)),
      func(func),
      format(std::move(format))
{
}

bool NoResponseTask::BuildRequest(APDURequest& request, uint8_t seq)
{
    request.SetControl(AppControlField(true, true, false, false, seq));
    request.SetFunction(this->func);
    auto writer = request.GetWriter();
    return format(writer);
}

IMasterTask::ResponseResult NoResponseTask::ProcessResponse(const APDUResponseHeader& /*header*/,
                                                            const ser4cpp::rseq_t& /*objects*/)
{
    // No response is expected; if we somehow receive one, just complete the task
    return ResponseResult::OK_FINAL;
}

} // namespace opendnp3
