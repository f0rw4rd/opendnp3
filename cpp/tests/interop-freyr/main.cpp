/*
 * Interoperability tests: opendnp3 vs FreyrSCADA DNP3.
 *
 * These tests verify that opendnp3 can communicate correctly with a
 * third-party DNP3 implementation (FreyrSCADA) over TCP localhost.
 *
 * The FreyrSCADA DNP3 library is proprietary software.  It is NOT
 * redistributed with opendnp3.  The Docker build downloads it at build
 * time solely for the purpose of interoperability testing.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
