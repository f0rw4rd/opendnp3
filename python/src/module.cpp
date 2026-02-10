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
    m.doc() = "Python bindings for opendnp3 -- the de facto reference implementation of IEEE-1815 (DNP3)";

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
