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

#include "opendnp3/app/AnalogCommandEvent.h"
#include "opendnp3/app/BinaryCommandEvent.h"
#include "opendnp3/app/Indexed.h"
#include "opendnp3/app/MeasurementTypes.h"
#include "opendnp3/app/OctetString.h"
#include "opendnp3/app/parsing/ICollection.h"
#include "opendnp3/master/CommandPointResult.h"
#include "opendnp3/master/CommandResultCallbackT.h"
#include "opendnp3/master/CommandSet.h"
#include "opendnp3/master/FileOperationResult.h"
#include "opendnp3/master/HeaderInfo.h"
#include "opendnp3/master/HeaderTypes.h"
#include "opendnp3/master/ICommandProcessor.h"
#include "opendnp3/master/ICommandTaskResult.h"
#include "opendnp3/master/IMaster.h"
#include "opendnp3/master/IMasterApplication.h"
#include "opendnp3/master/IMasterOperations.h"
#include "opendnp3/master/IMasterScan.h"
#include "opendnp3/master/ISOEHandler.h"
#include "opendnp3/master/ResponseInfo.h"
#include "opendnp3/master/TaskConfig.h"
#include "opendnp3/master/TaskInfo.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vector>

namespace py = pybind11;
using namespace opendnp3;

// Helper: convert ICollection<T> to std::vector<T>
template<class T> std::vector<T> ToVector(const ICollection<T>& coll)
{
    std::vector<T> items;
    items.reserve(coll.Count());
    coll.ForeachItem([&](const T& v) { items.push_back(v); });
    return items;
}

// Trampoline for ISOEHandler
// Each Process() override converts the ICollection to a std::vector before
// dispatching to the Python override so that Python receives a plain list.
class PySOEHandler : public ISOEHandler
{
public:
    using ISOEHandler::ISOEHandler;

    void BeginFragment(const ResponseInfo& info) override
    {
        PYBIND11_OVERRIDE_PURE(void, ISOEHandler, BeginFragment, info);
    }

    void EndFragment(const ResponseInfo& info) override
    {
        PYBIND11_OVERRIDE_PURE(void, ISOEHandler, EndFragment, info);
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<DoubleBitBinary>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<Counter>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<FrozenCounter>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<OctetString>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<TimeAndInterval>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryCommandEvent>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogCommandEvent>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogInputDeadband>>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void Process(const HeaderInfo& info, const ICollection<DNPTime>& values) override
    {
        auto items = ToVector(values);
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "Process");
        if (override_fn)
        {
            override_fn(info, items);
        }
    }

    void OnRawAPDU(const ResponseInfo& info, const uint8_t* data, size_t length) override
    {
        py::gil_scoped_acquire gil;
        py::function override_fn = py::get_override(this, "OnRawAPDU");
        if (override_fn)
            override_fn(info, py::bytes(reinterpret_cast<const char*>(data), length));
    }
};

// Trampoline for IMasterApplication
class PyMasterApplication : public IMasterApplication
{
public:
    using IMasterApplication::IMasterApplication;

    void OnReceiveIIN(const IINField& iin) override
    {
        PYBIND11_OVERRIDE(void, IMasterApplication, OnReceiveIIN, iin);
    }

    void OnTaskStart(MasterTaskType type, TaskId id) override
    {
        PYBIND11_OVERRIDE(void, IMasterApplication, OnTaskStart, type, id);
    }

    void OnTaskComplete(const TaskInfo& info) override
    {
        PYBIND11_OVERRIDE(void, IMasterApplication, OnTaskComplete, info);
    }

    void OnOpen() override
    {
        PYBIND11_OVERRIDE(void, IMasterApplication, OnOpen);
    }

    void OnClose() override
    {
        PYBIND11_OVERRIDE(void, IMasterApplication, OnClose);
    }

    bool AssignClassDuringStartup() override
    {
        PYBIND11_OVERRIDE(bool, IMasterApplication, AssignClassDuringStartup);
    }

    UTCTimestamp Now() override
    {
        PYBIND11_OVERRIDE_PURE(UTCTimestamp, IMasterApplication, Now);
    }
};

// Snapshot of ICommandTaskResult data, safe to use after the callback returns
struct CommandTaskResultSnapshot
{
    TaskCompletion summary;
    std::vector<CommandPointResult> results;

    static CommandTaskResultSnapshot From(const ICommandTaskResult& src)
    {
        CommandTaskResultSnapshot snap;
        snap.summary = src.summary;
        src.ForeachItem([&](const CommandPointResult& v) { snap.results.push_back(v); });
        return snap;
    }
};

void init_master(py::module_& m)
{
    // ResponseInfo
    py::class_<ResponseInfo>(m, "ResponseInfo", "Info about a response or unsolicited response")
        .def_readonly("unsolicited", &ResponseInfo::unsolicited)
        .def_readonly("fir", &ResponseInfo::fir)
        .def_readonly("fin", &ResponseInfo::fin);

    // TaskConfig
    py::class_<TaskConfig>(m, "TaskConfig", "Configuration for a master task")
        .def_static("Default", &TaskConfig::Default);

    // ICommandTaskResult (snapshot copy, safe to use outside callbacks)
    py::class_<CommandTaskResultSnapshot>(m, "ICommandTaskResult", "Result of a command task operation")
        .def_readonly("summary", &CommandTaskResultSnapshot::summary)
        .def(
            "to_list", [](const CommandTaskResultSnapshot& self) { return self.results; },
            "Get the results as a Python list");

    // ISOEHandler
    py::class_<ISOEHandler, PySOEHandler, std::shared_ptr<ISOEHandler>>(
        m, "ISOEHandler", "Sequence-of-events callback interface for receiving measurements from a master")
        .def(py::init<>())
        .def("BeginFragment", &ISOEHandler::BeginFragment, py::arg("info"))
        .def("EndFragment", &ISOEHandler::EndFragment, py::arg("info"));

    // IMasterApplication
    py::class_<IMasterApplication, PyMasterApplication, std::shared_ptr<IMasterApplication>>(
        m, "IMasterApplication", "Master application callback interface")
        .def(py::init<>())
        .def("OnReceiveIIN", &IMasterApplication::OnReceiveIIN, py::arg("iin"))
        .def("OnTaskStart", &IMasterApplication::OnTaskStart, py::arg("type"), py::arg("id"))
        .def("OnTaskComplete", &IMasterApplication::OnTaskComplete, py::arg("info"))
        .def("OnOpen", &IMasterApplication::OnOpen)
        .def("OnClose", &IMasterApplication::OnClose)
        .def("AssignClassDuringStartup", &IMasterApplication::AssignClassDuringStartup);

    // IMasterScan
    py::class_<IMasterScan, std::shared_ptr<IMasterScan>>(m, "IMasterScan", "Handle for a running scan")
        .def("Demand", &IMasterScan::Demand, py::call_guard<py::gil_scoped_release>());

    // IMaster (inherits IStack + IMasterOperations)
    py::class_<IMaster, std::shared_ptr<IMaster>>(m, "IMaster", "Running master session")
        .def("Enable", &IMaster::Enable, py::call_guard<py::gil_scoped_release>())
        .def("Disable", &IMaster::Disable, py::call_guard<py::gil_scoped_release>())
        .def("Shutdown", &IMaster::Shutdown, py::call_guard<py::gil_scoped_release>())
        .def("SetLogFilters", &IMaster::SetLogFilters, py::arg("filters"))
        .def("Scan", &IMaster::Scan, py::arg("headers"), py::arg("soe_handler"),
             py::arg("config") = TaskConfig::Default(), "Initiate a one-shot scan using custom headers",
             py::call_guard<py::gil_scoped_release>())
        .def("ScanClasses", &IMaster::ScanClasses, py::arg("field"), py::arg("soe_handler"),
             py::arg("config") = TaskConfig::Default(), "Initiate a one-shot class-based scan",
             py::call_guard<py::gil_scoped_release>())
        .def("AddClassScan", &IMaster::AddClassScan, py::arg("field"), py::arg("period"), py::arg("soe_handler"),
             py::arg("config") = TaskConfig::Default(), "Add a recurring class scan",
             py::call_guard<py::gil_scoped_release>())
        .def("Write", &IMaster::Write, py::arg("value"), py::arg("index"), py::arg("config") = TaskConfig::Default(),
             py::call_guard<py::gil_scoped_release>())
        .def("Restart", &IMaster::Restart, py::arg("op"), py::arg("callback"),
             py::arg("config") = TaskConfig::Default(), py::call_guard<py::gil_scoped_release>())
        .def("PerformFunction", &IMaster::PerformFunction, py::arg("name"), py::arg("func"), py::arg("headers"),
             py::arg("config") = TaskConfig::Default(), py::call_guard<py::gil_scoped_release>())
        .def("Freeze", &IMaster::Freeze, py::arg("type"), py::arg("headers"), py::arg("config") = TaskConfig::Default(),
             "Issue a freeze request to the outstation", py::call_guard<py::gil_scoped_release>())
        .def("AddScan", &IMaster::AddScan, py::arg("period"), py::arg("headers"), py::arg("soe_handler"),
             py::arg("config") = TaskConfig::Default(), "Add a recurring user-defined scan from a vector of headers",
             py::call_guard<py::gil_scoped_release>())
        .def("AddAllObjectsScan", &IMaster::AddAllObjectsScan, py::arg("gvId"), py::arg("period"),
             py::arg("soe_handler"), py::arg("config") = TaskConfig::Default(),
             "Add a recurring all-objects scan for a group/variation", py::call_guard<py::gil_scoped_release>())
        .def("AddRangeScan", &IMaster::AddRangeScan, py::arg("gvId"), py::arg("start"), py::arg("stop"),
             py::arg("period"), py::arg("soe_handler"), py::arg("config") = TaskConfig::Default(),
             "Add a recurring range-based scan", py::call_guard<py::gil_scoped_release>())
        .def("ScanAllObjects", &IMaster::ScanAllObjects, py::arg("gvId"), py::arg("soe_handler"),
             py::arg("config") = TaskConfig::Default(), "Initiate a single all-objects scan for a group/variation",
             py::call_guard<py::gil_scoped_release>())
        .def("ScanRange", &IMaster::ScanRange, py::arg("gvId"), py::arg("start"), py::arg("stop"),
             py::arg("soe_handler"), py::arg("config") = TaskConfig::Default(), "Initiate a single range-based scan",
             py::call_guard<py::gil_scoped_release>())
        .def(
            "SelectAndOperate",
            [](IMaster& self, const ControlRelayOutputBlock& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.SelectAndOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Select and operate a CROB at the given index")
        .def(
            "SelectAndOperate",
            [](IMaster& self, const AnalogOutputInt16& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.SelectAndOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "SelectAndOperate",
            [](IMaster& self, const AnalogOutputInt32& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.SelectAndOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "SelectAndOperate",
            [](IMaster& self, const AnalogOutputFloat32& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.SelectAndOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "SelectAndOperate",
            [](IMaster& self, const AnalogOutputDouble64& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.SelectAndOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "DirectOperate",
            [](IMaster& self, const ControlRelayOutputBlock& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.DirectOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Direct operate a CROB at the given index")
        .def(
            "DirectOperate",
            [](IMaster& self, const AnalogOutputInt16& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.DirectOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "DirectOperate",
            [](IMaster& self, const AnalogOutputInt32& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.DirectOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "DirectOperate",
            [](IMaster& self, const AnalogOutputFloat32& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.DirectOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        .def(
            "DirectOperate",
            [](IMaster& self, const AnalogOutputDouble64& command, uint16_t index, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const ICommandTaskResult& result) {
                    auto snap = CommandTaskResultSnapshot::From(result);
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.DirectOperate(command, index, safe_cb, config);
            },
            py::arg("command"), py::arg("index"), py::arg("callback"), py::arg("config") = TaskConfig::Default())
        // ===== File Transfer Methods =====
        .def(
            "ReadFile",
            [](IMaster& self, const std::string& filename, py::function callback, const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const FileReadResult& result) {
                    auto snap = result; // copy before acquiring GIL
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.ReadFile(filename, safe_cb, config);
            },
            py::arg("filename"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Read a file from the outstation", py::call_guard<py::gil_scoped_release>())
        .def(
            "WriteFile",
            [](IMaster& self, const std::string& filename, py::bytes data, FilePermissions permissions,
               py::function callback, const TaskConfig& config) {
                std::string raw = data;
                std::vector<uint8_t> vec(raw.begin(), raw.end());
                auto safe_cb = [cb = std::move(callback)](const FileWriteResult& result) {
                    auto snap = result;
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.WriteFile(filename, vec, permissions, safe_cb, config);
            },
            py::arg("filename"), py::arg("data"), py::arg("permissions"), py::arg("callback"),
            py::arg("config") = TaskConfig::Default(), "Write a file to the outstation",
            py::call_guard<py::gil_scoped_release>())
        .def(
            "DeleteFile",
            [](IMaster& self, const std::string& filename, py::function callback, const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const FileOperationResult& result) {
                    auto snap = result;
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.DeleteFile(filename, safe_cb, config);
            },
            py::arg("filename"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Delete a file on the outstation", py::call_guard<py::gil_scoped_release>())
        .def(
            "GetFileInfo",
            [](IMaster& self, const std::string& filename, py::function callback, const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const FileInfoResult& result) {
                    auto snap = result;
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.GetFileInfo(filename, safe_cb, config);
            },
            py::arg("filename"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Get file information from the outstation", py::call_guard<py::gil_scoped_release>())
        .def(
            "ReadDirectory",
            [](IMaster& self, const std::string& path, py::function callback, const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const DirectoryReadResult& result) {
                    auto snap = result;
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.ReadDirectory(path, safe_cb, config);
            },
            py::arg("path"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Read a directory listing from the outstation", py::call_guard<py::gil_scoped_release>())
        .def(
            "AbortFile",
            [](IMaster& self, uint32_t fileHandle, py::function callback, const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const FileOperationResult& result) {
                    auto snap = result;
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.AbortFile(fileHandle, safe_cb, config);
            },
            py::arg("fileHandle"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Abort an in-progress file transfer", py::call_guard<py::gil_scoped_release>())
        .def(
            "AuthenticateFile",
            [](IMaster& self, const std::string& username, const std::string& password, py::function callback,
               const TaskConfig& config) {
                auto safe_cb = [cb = std::move(callback)](const FileAuthResult_t& result) {
                    auto snap = result;
                    py::gil_scoped_acquire gil;
                    cb(snap);
                };
                self.AuthenticateFile(username, password, safe_cb, config);
            },
            py::arg("username"), py::arg("password"), py::arg("callback"), py::arg("config") = TaskConfig::Default(),
            "Authenticate for file transfer operations", py::call_guard<py::gil_scoped_release>());
}
