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

#include "opendnp3/DNP3Manager.h"
#include "opendnp3/channel/IChannel.h"
#include "opendnp3/channel/IChannelListener.h"
#include "opendnp3/channel/TLSConfig.h"
#include "opendnp3/logging/ILogHandler.h"
#include "opendnp3/outstation/IFileHandler.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace opendnp3;

// Trampoline for ILogHandler so Python can subclass it
class PyLogHandler : public ILogHandler
{
public:
    using ILogHandler::ILogHandler;

    void log(ModuleId module, const char* id, LogLevel level, char const* location, char const* message) override
    {
        PYBIND11_OVERRIDE_PURE(void, ILogHandler, log, module, id, level, location, message);
    }
};

// Trampoline for IChannelListener so Python can subclass it
class PyChannelListener : public IChannelListener
{
public:
    using IChannelListener::IChannelListener;

    void OnStateChange(ChannelState state) override
    {
        PYBIND11_OVERRIDE_PURE(void, IChannelListener, OnStateChange, state);
    }
};

void init_channel(py::module_& m)
{
    // ModuleId
    py::class_<ModuleId>(m, "ModuleId")
        .def(py::init<>())
        .def(py::init<int32_t>(), py::arg("level"))
        .def_readwrite("value", &ModuleId::value);

    // ILogHandler
    py::class_<ILogHandler, PyLogHandler, std::shared_ptr<ILogHandler>>(m, "ILogHandler",
                                                                        "Callback interface for log messages")
        .def(py::init<>())
        .def("log", &ILogHandler::log, py::arg("module"), py::arg("id"), py::arg("level"), py::arg("location"),
             py::arg("message"));

    // IChannelListener
    py::class_<IChannelListener, PyChannelListener, std::shared_ptr<IChannelListener>>(
        m, "IChannelListener", "Callback interface for channel state changes")
        .def(py::init<>())
        .def("OnStateChange", &IChannelListener::OnStateChange, py::arg("state"));

    // IChannel
    py::class_<IChannel, std::shared_ptr<IChannel>>(m, "IChannel", "Communication channel")
        .def("GetLogFilters", &IChannel::GetLogFilters)
        .def("SetLogFilters", &IChannel::SetLogFilters, py::arg("filters"))
        .def("AddMaster", &IChannel::AddMaster, py::arg("id"), py::arg("SOEHandler"), py::arg("application"),
             py::arg("config"), "Add a master session to this channel", py::call_guard<py::gil_scoped_release>())
        .def(
            "AddOutstation",
            [](IChannel& self, const std::string& id, std::shared_ptr<ICommandHandler> commandHandler,
               std::shared_ptr<IOutstationApplication> application, const OutstationStackConfig& config,
               std::shared_ptr<IFileHandler> fileHandler) {
                py::gil_scoped_release release;
                return self.AddOutstation(id, commandHandler, application, config, fileHandler);
            },
            py::arg("id"), py::arg("commandHandler"), py::arg("application"), py::arg("config"),
            py::arg("fileHandler") = std::shared_ptr<IFileHandler>(nullptr),
            "Add an outstation session to this channel")
        .def("Shutdown", &IChannel::Shutdown, py::call_guard<py::gil_scoped_release>());

    // TLSConfig
    py::class_<TLSConfig>(m, "TLSConfig", "TLS configuration information")
        .def(py::init<const std::string&, const std::string&, const std::string&, bool, bool, bool, bool,
                      const std::string&>(),
             py::arg("peerCertFilePath"), py::arg("localCertFilePath"), py::arg("privateKeyFilePath"),
             py::arg("allowTLSv10") = false, py::arg("allowTLSv11") = false, py::arg("allowTLSv12") = true,
             py::arg("allowTLSv13") = true, py::arg("cipherList") = "")
        .def_readwrite("peerCertFilePath", &TLSConfig::peerCertFilePath)
        .def_readwrite("localCertFilePath", &TLSConfig::localCertFilePath)
        .def_readwrite("privateKeyFilePath", &TLSConfig::privateKeyFilePath)
        .def_readwrite("allowTLSv10", &TLSConfig::allowTLSv10)
        .def_readwrite("allowTLSv11", &TLSConfig::allowTLSv11)
        .def_readwrite("allowTLSv12", &TLSConfig::allowTLSv12)
        .def_readwrite("allowTLSv13", &TLSConfig::allowTLSv13)
        .def_readwrite("cipherList", &TLSConfig::cipherList)
        .def_property(
            "verifyCallback",
            [](const TLSConfig& self) -> py::object {
                return self.verifyCallback ? py::cast(self.verifyCallback) : py::none();
            },
            [](TLSConfig& self, py::object cb) {
                if (cb.is_none())
                {
                    self.verifyCallback = nullptr;
                    return;
                }
                py::function py_cb = py::reinterpret_borrow<py::function>(cb);
                self.verifyCallback = [py_cb](bool preverified, int depth, const std::string& subject,
                                              const std::string& certDER) -> bool {
                    py::gil_scoped_acquire gil;
                    return py_cb(preverified, depth, subject, py::bytes(certDER.data(), certDER.size())).cast<bool>();
                };
            },
            "Optional callback(preverified: bool, depth: int, subject: str, cert_der: bytes) -> bool");

    // DNP3Manager
    py::class_<DNP3Manager>(m, "DNP3Manager", "Root DNP3 object used to create channels and sessions")
        .def(py::init<uint32_t, std::shared_ptr<ILogHandler>>(), py::arg("concurrencyHint"),
             py::arg("handler") = std::shared_ptr<ILogHandler>())
        .def("Shutdown", &DNP3Manager::Shutdown, py::call_guard<py::gil_scoped_release>())
        .def("AddTCPClient", &DNP3Manager::AddTCPClient, py::arg("id"), py::arg("levels"), py::arg("retry"),
             py::arg("hosts"), py::arg("local"), py::arg("listener"), "Add a persistent TCP client channel",
             py::call_guard<py::gil_scoped_release>())
        .def("AddTCPServer", &DNP3Manager::AddTCPServer, py::arg("id"), py::arg("levels"), py::arg("mode"),
             py::arg("endpoint"), py::arg("listener"), "Add a persistent TCP server channel",
             py::call_guard<py::gil_scoped_release>())
        .def("AddTLSClient", &DNP3Manager::AddTLSClient, py::arg("id"), py::arg("levels"), py::arg("retry"),
             py::arg("hosts"), py::arg("local"), py::arg("config"), py::arg("listener"),
             "Add a persistent TLS client channel", py::call_guard<py::gil_scoped_release>())
        .def("AddTLSServer", &DNP3Manager::AddTLSServer, py::arg("id"), py::arg("levels"), py::arg("mode"),
             py::arg("endpoint"), py::arg("config"), py::arg("listener"), "Add a persistent TLS server channel",
             py::call_guard<py::gil_scoped_release>())
        .def("AddUDPChannel", &DNP3Manager::AddUDPChannel, py::arg("id"), py::arg("levels"), py::arg("retry"),
             py::arg("localEndpoint"), py::arg("remoteEndpoint"), py::arg("listener"), "Add a persistent UDP channel",
             py::call_guard<py::gil_scoped_release>())
        .def("AddSerial", &DNP3Manager::AddSerial, py::arg("id"), py::arg("levels"), py::arg("retry"),
             py::arg("settings"), py::arg("listener"), "Add a persistent serial channel",
             py::call_guard<py::gil_scoped_release>());
}
