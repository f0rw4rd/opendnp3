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

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Forward declarations for init functions in each binding file
void init_enums(py::module_& m);
void init_types(py::module_& m);
void init_channel(py::module_& m);
void init_master(py::module_& m);
void init_outstation(py::module_& m);

PYBIND11_MODULE(_opendnp3, m)
{
    m.doc() = "Python bindings for yadnp3 -- Yet Another opendnp3 fork (IEEE-1815 DNP3 protocol)";

    // Order matters: enums and types must be registered before they are
    // referenced by channel/master/outstation bindings.
    // outstation must come before channel because IFileHandler is used
    // as a default parameter in AddOutstation.
    init_enums(m);
    init_types(m);
    init_master(m);
    init_outstation(m);
    init_channel(m);
}
