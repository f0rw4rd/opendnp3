/*
 * Copyright 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * This file is part of an experimental fork of opendnp3.
 * See the NOTICE file for upstream copyright attribution.
 */

#include "opendnp3/outstation/ICommandHandler.h"
#include "opendnp3/outstation/IFileHandler.h"
#include "opendnp3/outstation/IOutstation.h"
#include "opendnp3/outstation/IOutstationApplication.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace opendnp3;

// Trampoline for ICommandHandler
class PyCommandHandler : public ICommandHandler
{
public:
    using ICommandHandler::ICommandHandler;

    void Begin() override
    {
        PYBIND11_OVERRIDE_PURE(void, ICommandHandler, Begin);
    }
    void End() override
    {
        PYBIND11_OVERRIDE_PURE(void, ICommandHandler, End);
    }

    CommandStatus Select(const ControlRelayOutputBlock& command, uint16_t index) override
    {
        PYBIND11_OVERRIDE_PURE(CommandStatus, ICommandHandler, Select, command, index);
    }

    CommandStatus Operate(const ControlRelayOutputBlock& command,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Operate");
        if (override_fn)
            return override_fn(command, index, py::cast(handler, py::return_value_policy::reference), opType)
                .cast<CommandStatus>();
        throw std::runtime_error("Tried to call pure virtual function \"ICommandHandler::Operate\"");
    }

    CommandStatus Select(const AnalogOutputInt16& command, uint16_t index) override
    {
        PYBIND11_OVERRIDE_PURE(CommandStatus, ICommandHandler, Select, command, index);
    }

    CommandStatus Operate(const AnalogOutputInt16& command,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Operate");
        if (override_fn)
            return override_fn(command, index, py::cast(handler, py::return_value_policy::reference), opType)
                .cast<CommandStatus>();
        throw std::runtime_error("Tried to call pure virtual function \"ICommandHandler::Operate\"");
    }

    CommandStatus Select(const AnalogOutputInt32& command, uint16_t index) override
    {
        PYBIND11_OVERRIDE_PURE(CommandStatus, ICommandHandler, Select, command, index);
    }

    CommandStatus Operate(const AnalogOutputInt32& command,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Operate");
        if (override_fn)
            return override_fn(command, index, py::cast(handler, py::return_value_policy::reference), opType)
                .cast<CommandStatus>();
        throw std::runtime_error("Tried to call pure virtual function \"ICommandHandler::Operate\"");
    }

    CommandStatus Select(const AnalogOutputFloat32& command, uint16_t index) override
    {
        PYBIND11_OVERRIDE_PURE(CommandStatus, ICommandHandler, Select, command, index);
    }

    CommandStatus Operate(const AnalogOutputFloat32& command,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Operate");
        if (override_fn)
            return override_fn(command, index, py::cast(handler, py::return_value_policy::reference), opType)
                .cast<CommandStatus>();
        throw std::runtime_error("Tried to call pure virtual function \"ICommandHandler::Operate\"");
    }

    CommandStatus Select(const AnalogOutputDouble64& command, uint16_t index) override
    {
        PYBIND11_OVERRIDE_PURE(CommandStatus, ICommandHandler, Select, command, index);
    }

    CommandStatus Operate(const AnalogOutputDouble64& command,
                          uint16_t index,
                          IUpdateHandler& handler,
                          OperateType opType) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Operate");
        if (override_fn)
            return override_fn(command, index, py::cast(handler, py::return_value_policy::reference), opType)
                .cast<CommandStatus>();
        throw std::runtime_error("Tried to call pure virtual function \"ICommandHandler::Operate\"");
    }
};

// Trampoline for IOutstationApplication
class PyOutstationApplication : public IOutstationApplication
{
public:
    using IOutstationApplication::IOutstationApplication;

    bool SupportsWriteAbsoluteTime() override
    {
        PYBIND11_OVERRIDE(bool, IOutstationApplication, SupportsWriteAbsoluteTime);
    }

    bool WriteAbsoluteTime(const UTCTimestamp& timestamp) override
    {
        PYBIND11_OVERRIDE(bool, IOutstationApplication, WriteAbsoluteTime, timestamp);
    }

    bool SupportsAssignClass() override
    {
        PYBIND11_OVERRIDE(bool, IOutstationApplication, SupportsAssignClass);
    }

    ApplicationIIN GetApplicationIIN() const override
    {
        PYBIND11_OVERRIDE(ApplicationIIN, IOutstationApplication, GetApplicationIIN);
    }

    RestartMode ColdRestartSupport() const override
    {
        PYBIND11_OVERRIDE(RestartMode, IOutstationApplication, ColdRestartSupport);
    }

    RestartMode WarmRestartSupport() const override
    {
        PYBIND11_OVERRIDE(RestartMode, IOutstationApplication, WarmRestartSupport);
    }

    uint16_t ColdRestart() override
    {
        PYBIND11_OVERRIDE(uint16_t, IOutstationApplication, ColdRestart);
    }

    uint16_t WarmRestart() override
    {
        PYBIND11_OVERRIDE(uint16_t, IOutstationApplication, WarmRestart);
    }

    DNPTime Now() override
    {
        PYBIND11_OVERRIDE(DNPTime, IOutstationApplication, Now);
    }
};

// Trampoline for IFileHandler
class PyFileHandler : public IFileHandler
{
public:
    using IFileHandler::IFileHandler;

    FileCommandResult GetFileInfo(const std::string& filename) override
    {
        PYBIND11_OVERRIDE_PURE(FileCommandResult, IFileHandler, GetFileInfo, filename);
    }

    FileOpenResult OpenFile(const std::string& filename,
                            uint32_t authKey,
                            FilePermissions permissions,
                            FileMode mode,
                            uint16_t maxBlockSize,
                            uint16_t requestId) override
    {
        PYBIND11_OVERRIDE_PURE(FileOpenResult, IFileHandler, OpenFile, filename, authKey, permissions, mode,
                               maxBlockSize, requestId);
    }

    FileBlockResult ReadBlock(uint32_t fileHandle, uint32_t blockNum) override
    {
        PYBIND11_OVERRIDE_PURE(FileBlockResult, IFileHandler, ReadBlock, fileHandle, blockNum);
    }

    FileStatus WriteBlock(
        uint32_t fileHandle, uint32_t blockNum, bool lastBlock, const uint8_t* data, size_t size) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "WriteBlock");
        if (override_fn)
            return override_fn(fileHandle, blockNum, lastBlock, py::bytes(reinterpret_cast<const char*>(data), size))
                .cast<FileStatus>();
        throw std::runtime_error("Tried to call pure virtual function \"IFileHandler::WriteBlock\"");
    }

    FileStatus CloseFile(uint32_t fileHandle, uint16_t requestId) override
    {
        PYBIND11_OVERRIDE_PURE(FileStatus, IFileHandler, CloseFile, fileHandle, requestId);
    }

    FileStatus DeleteFile(const std::string& filename) override
    {
        PYBIND11_OVERRIDE_PURE(FileStatus, IFileHandler, DeleteFile, filename);
    }

    void AbortFile(uint32_t fileHandle) override
    {
        PYBIND11_OVERRIDE_PURE(void, IFileHandler, AbortFile, fileHandle);
    }

    FileAuthResult AuthenticateFile(const std::string& username, const std::string& password) override
    {
        PYBIND11_OVERRIDE_PURE(FileAuthResult, IFileHandler, AuthenticateFile, username, password);
    }
};

void init_outstation(py::module_& m)
{
    // ICommandHandler
    py::class_<ICommandHandler, PyCommandHandler, std::shared_ptr<ICommandHandler>>(
        m, "ICommandHandler", "Callback interface for handling command requests at an outstation")
        .def(py::init<>())
        .def("Begin", &ICommandHandler::Begin)
        .def("End", &ICommandHandler::End);

    // IOutstationApplication
    py::class_<IOutstationApplication, PyOutstationApplication, std::shared_ptr<IOutstationApplication>>(
        m, "IOutstationApplication", "Outstation application callback interface")
        .def(py::init<>())
        .def("SupportsWriteAbsoluteTime", &IOutstationApplication::SupportsWriteAbsoluteTime)
        .def("WriteAbsoluteTime", &IOutstationApplication::WriteAbsoluteTime, py::arg("timestamp"))
        .def("SupportsAssignClass", &IOutstationApplication::SupportsAssignClass)
        .def("GetApplicationIIN", &IOutstationApplication::GetApplicationIIN)
        .def("ColdRestartSupport", &IOutstationApplication::ColdRestartSupport)
        .def("WarmRestartSupport", &IOutstationApplication::WarmRestartSupport)
        .def("ColdRestart", &IOutstationApplication::ColdRestart)
        .def("WarmRestart", &IOutstationApplication::WarmRestart);

    // IFileHandler
    py::class_<IFileHandler, PyFileHandler, std::shared_ptr<IFileHandler>>(
        m, "IFileHandler", "Callback interface for outstation file transfer operations")
        .def(py::init<>())
        .def("GetFileInfo", &IFileHandler::GetFileInfo, py::arg("filename"))
        .def("OpenFile", &IFileHandler::OpenFile, py::arg("filename"), py::arg("authKey"), py::arg("permissions"),
             py::arg("mode"), py::arg("maxBlockSize"), py::arg("requestId"))
        .def("ReadBlock", &IFileHandler::ReadBlock, py::arg("fileHandle"), py::arg("blockNum"))
        .def("CloseFile", &IFileHandler::CloseFile, py::arg("fileHandle"), py::arg("requestId"))
        .def("DeleteFile", &IFileHandler::DeleteFile, py::arg("filename"))
        .def("AbortFile", &IFileHandler::AbortFile, py::arg("fileHandle"))
        .def("AuthenticateFile", &IFileHandler::AuthenticateFile, py::arg("username"), py::arg("password"));

    // IOutstation
    py::class_<IOutstation, std::shared_ptr<IOutstation>>(m, "IOutstation", "Running outstation session")
        .def("Enable", &IOutstation::Enable, py::call_guard<py::gil_scoped_release>())
        .def("Disable", &IOutstation::Disable, py::call_guard<py::gil_scoped_release>())
        .def("Shutdown", &IOutstation::Shutdown, py::call_guard<py::gil_scoped_release>())
        .def("SetLogFilters", &IOutstation::SetLogFilters, py::arg("filters"))
        .def("SetRestartIIN", &IOutstation::SetRestartIIN, py::call_guard<py::gil_scoped_release>())
        .def("Apply", &IOutstation::Apply, py::arg("updates"), "Apply measurement updates to the outstation database",
             py::call_guard<py::gil_scoped_release>());
}
