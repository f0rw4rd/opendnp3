/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Modified 2024-2026 f0rw4rd (experimental fork)
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

#include "master/MasterSessionStack.h"

#include "link/LinkSession.h"
#include "master/HeaderConversions.h"

#include <utility>

namespace opendnp3
{
std::shared_ptr<MasterSessionStack> MasterSessionStack::Create(const Logger& logger,
                                                               const std::shared_ptr<exe4cpp::StrandExecutor>& executor,
                                                               const std::shared_ptr<ISOEHandler>& SOEHandler,
                                                               const std::shared_ptr<IMasterApplication>& application,
                                                               const std::shared_ptr<IMasterScheduler>& scheduler,
                                                               const std::shared_ptr<LinkSession>& session,
                                                               ILinkTx& linktx,
                                                               const MasterStackConfig& config)
{
    return std::make_shared<MasterSessionStack>(logger, executor, SOEHandler, application, scheduler, session, linktx,
                                                config);
}

MasterSessionStack::MasterSessionStack(const Logger& logger,
                                       const std::shared_ptr<exe4cpp::StrandExecutor>& executor,
                                       const std::shared_ptr<ISOEHandler>& SOEHandler,
                                       const std::shared_ptr<IMasterApplication>& application,
                                       const std::shared_ptr<IMasterScheduler>& scheduler,
                                       std::shared_ptr<LinkSession> session,
                                       ILinkTx& linktx,
                                       const MasterStackConfig& config)
    : executor(executor),
      scheduler(scheduler),
      session(std::move(session)),
      stack(logger, executor, application, config.master.maxRxFragSize, LinkLayerConfig(config.link, false)),
      context(MContext::Create(Addresses(config.link.LocalAddr, config.link.RemoteAddr),
                               logger,
                               executor,
                               stack.transport,
                               SOEHandler,
                               application,
                               scheduler,
                               config.master))
{
    stack.link->SetRouter(linktx);
    stack.transport->SetAppLayer(*context);
}

void MasterSessionStack::OnLowerLayerUp()
{
    stack.link->OnLowerLayerUp();
}

void MasterSessionStack::OnLowerLayerDown()
{
    stack.link->OnLowerLayerDown();
}

bool MasterSessionStack::OnFrame(const LinkHeaderFields& header, const ser4cpp::rseq_t& userdata)
{
    return stack.link->OnFrame(header, userdata);
}

void MasterSessionStack::OnTxReady()
{
    this->stack.link->OnTxReady();
}

void MasterSessionStack::SetLogFilters(const LogLevels& filters)
{
    auto set = [this, filters]() { this->session->SetLogFilters(filters); };

    this->executor->post(set);
}

void MasterSessionStack::BeginShutdown()
{
    auto shutdown = [this]() {
        if (scheduler)
        {
            scheduler->Shutdown();
            scheduler.reset();
        }

        // now we can release the socket session
        if (session)
        {
            session->Shutdown();
            session.reset();
        }
    };

    executor->post(shutdown);
}

StackStatistics MasterSessionStack::GetStackStatistics()
{
    auto get = [self = shared_from_this()]() -> StackStatistics { return self->CreateStatistics(); };
    return executor->return_from<StackStatistics>(get);
}

std::shared_ptr<IMasterScan> MasterSessionStack::AddScan(TimeDuration period,
                                                         const std::vector<Header>& headers,
                                                         std::shared_ptr<ISOEHandler> soe_handler,
                                                         const TaskConfig& config)
{
    auto builder = ConvertToLambda(headers);
    auto get = [self = shared_from_this(), soe_handler, period, builder, config]() {
        return self->context->AddScan(period, builder, std::move(soe_handler), config);
    };
    return MasterScan::Create(executor->return_from<std::shared_ptr<IMasterTask>>(get), this->scheduler);
}

std::shared_ptr<IMasterScan> MasterSessionStack::AddAllObjectsScan(GroupVariationID gvId,
                                                                   TimeDuration period,
                                                                   std::shared_ptr<ISOEHandler> soe_handler,
                                                                   const TaskConfig& config)
{
    auto get = [self = shared_from_this(), soe_handler, gvId, period, config] {
        return self->context->AddAllObjectsScan(gvId, period, std::move(soe_handler), config);
    };
    return MasterScan::Create(executor->return_from<std::shared_ptr<IMasterTask>>(get), this->scheduler);
}

std::shared_ptr<IMasterScan> MasterSessionStack::AddClassScan(const ClassField& field,
                                                              TimeDuration period,
                                                              std::shared_ptr<ISOEHandler> soe_handler,
                                                              const TaskConfig& config)
{
    auto get = [self = shared_from_this(), soe_handler, field, period, config] {
        return self->context->AddClassScan(field, period, std::move(soe_handler), config);
    };
    return MasterScan::Create(executor->return_from<std::shared_ptr<IMasterTask>>(get), this->scheduler);
}

std::shared_ptr<IMasterScan> MasterSessionStack::AddRangeScan(GroupVariationID gvId,
                                                              uint16_t start,
                                                              uint16_t stop,
                                                              TimeDuration period,
                                                              std::shared_ptr<ISOEHandler> soe_handler,
                                                              const TaskConfig& config)
{
    auto get = [self = shared_from_this(), soe_handler, gvId, start, stop, period, config] {
        return self->context->AddRangeScan(gvId, start, stop, period, std::move(soe_handler), config);
    };
    return MasterScan::Create(executor->return_from<std::shared_ptr<IMasterTask>>(get), this->scheduler);
}

void MasterSessionStack::Scan(const std::vector<Header>& headers,
                              std::shared_ptr<ISOEHandler> soe_handler,
                              const TaskConfig& config)
{
    auto builder = ConvertToLambda(headers);
    auto action = [self = shared_from_this(), soe_handler, builder, config]() -> void {
        self->context->Scan(builder, std::move(soe_handler), config);
    };
    return executor->post(action);
}

void MasterSessionStack::ScanAllObjects(GroupVariationID gvId,
                                        std::shared_ptr<ISOEHandler> soe_handler,
                                        const TaskConfig& config)
{
    auto action = [self = shared_from_this(), soe_handler, gvId, config]() -> void {
        self->context->ScanAllObjects(gvId, std::move(soe_handler), config);
    };
    return executor->post(action);
}

void MasterSessionStack::ScanClasses(const ClassField& field,
                                     std::shared_ptr<ISOEHandler> soe_handler,
                                     const TaskConfig& config)
{
    auto action = [self = shared_from_this(), soe_handler, field, config]() -> void {
        self->context->ScanClasses(field, std::move(soe_handler), config);
    };
    return executor->post(action);
}

void MasterSessionStack::ScanRange(GroupVariationID gvId,
                                   uint16_t start,
                                   uint16_t stop,
                                   std::shared_ptr<ISOEHandler> soe_handler,
                                   const TaskConfig& config)
{
    auto action = [self = shared_from_this(), soe_handler, gvId, start, stop, config]() -> void {
        self->context->ScanRange(gvId, start, stop, std::move(soe_handler), config);
    };
    return executor->post(action);
}

void MasterSessionStack::Write(const TimeAndInterval& value, uint16_t index, const TaskConfig& config)
{
    auto action
        = [self = shared_from_this(), value, index, config]() -> void { self->context->Write(value, index, config); };
    return executor->post(action);
}

void MasterSessionStack::Restart(RestartType op, const RestartOperationCallbackT& callback, TaskConfig config)
{
    auto action
        = [self = shared_from_this(), op, callback, config]() -> void { self->context->Restart(op, callback, config); };
    return executor->post(action);
}

void MasterSessionStack::PerformFunction(const std::string& name,
                                         FunctionCode func,
                                         const std::vector<Header>& headers,
                                         const TaskConfig& config)
{
    auto builder = ConvertToLambda(headers);
    auto action = [self = shared_from_this(), name, func, builder, config]() -> void {
        self->context->PerformFunction(name, func, builder, config);
    };
    return executor->post(action);
}

void MasterSessionStack::Freeze(FreezeType type, const std::vector<Header>& headers, const TaskConfig& config)
{
    auto builder = ConvertToLambda(headers);
    auto action = [self = shared_from_this(), type, builder, config]() -> void {
        self->context->Freeze(type, builder, config);
    };
    return executor->post(action);
}

void MasterSessionStack::ReadFile(const std::string& filename,
                                  const FileReadCallbackT& callback,
                                  const TaskConfig& config)
{
    auto action = [self = shared_from_this(), filename, callback, config]() -> void {
        self->context->ReadFile(filename, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::GetFileInfo(const std::string& filename,
                                     const FileInfoCallbackT& callback,
                                     const TaskConfig& config)
{
    auto action = [self = shared_from_this(), filename, callback, config]() -> void {
        self->context->GetFileInfo(filename, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::DeleteFile(const std::string& filename,
                                    const FileOperationCallbackT& callback,
                                    const TaskConfig& config)
{
    auto action = [self = shared_from_this(), filename, callback, config]() -> void {
        self->context->DeleteFile(filename, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::WriteFile(const std::string& filename,
                                   const std::vector<uint8_t>& data,
                                   FilePermissions permissions,
                                   const FileWriteCallbackT& callback,
                                   const TaskConfig& config)
{
    auto action = [self = shared_from_this(), filename, data, permissions, callback, config]() -> void {
        self->context->WriteFile(filename, data, permissions, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::ReadDirectory(const std::string& directoryPath,
                                       const DirectoryReadCallbackT& callback,
                                       const TaskConfig& config)
{
    auto action = [self = shared_from_this(), directoryPath, callback, config]() -> void {
        self->context->ReadDirectory(directoryPath, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::AbortFile(uint32_t fileHandle,
                                   const FileOperationCallbackT& callback,
                                   const TaskConfig& config)
{
    auto action = [self = shared_from_this(), fileHandle, callback, config]() -> void {
        self->context->AbortFile(fileHandle, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::AuthenticateFile(const std::string& username,
                                          const std::string& password,
                                          const FileAuthCallbackT& callback,
                                          const TaskConfig& config)
{
    auto action = [self = shared_from_this(), username, password, callback, config]() -> void {
        self->context->AuthenticateFile(username, password, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::WriteDeadBands(const std::vector<Indexed<AnalogInputDeadband>>& deadBands,
                                        const FileOperationCallbackT& callback,
                                        const TaskConfig& config)
{
    auto action = [self = shared_from_this(), deadBands, callback, config]() -> void {
        self->context->WriteDeadBands(deadBands, callback, config);
    };
    return executor->post(action);
}

void MasterSessionStack::CheckLinkStatus(const std::function<void(bool)>& callback)
{
    auto action = [self = shared_from_this(), callback]() -> void { self->stack.link->CheckLinkStatus(callback); };
    return executor->post(action);
}

/// --- ICommandProcessor ---

void MasterSessionStack::SelectAndOperate(CommandSet&& commands,
                                          const CommandResultCallbackT& callback,
                                          const TaskConfig& config)
{
    // this is required b/c move capture not supported in C++11
    auto set = std::make_shared<CommandSet>(std::move(commands));

    auto action = [self = shared_from_this(), set, config, callback]() -> void {
        self->context->SelectAndOperate(std::move(*set), callback, config);
    };
    executor->post(action);
}

void MasterSessionStack::DirectOperate(CommandSet&& commands,
                                       const CommandResultCallbackT& callback,
                                       const TaskConfig& config)
{
    // this is required b/c move capture not supported in C++11
    auto set = std::make_shared<CommandSet>(std::move(commands));

    auto action = [self = shared_from_this(), set, config, callback]() -> void {
        self->context->DirectOperate(std::move(*set), callback, config);
    };
    executor->post(action);
}

StackStatistics MasterSessionStack::CreateStatistics() const
{
    return StackStatistics(this->stack.link->GetStatistics(), this->stack.transport->GetStatistics());
}

} // namespace opendnp3
