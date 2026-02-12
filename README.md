# yadnp3

**Yet Another opendnp3 fork** -- an experimental fork of [opendnp3](https://github.com/dnp3/opendnp3), the de facto reference implementation of the **DNP3 (IEEE-1815)** protocol stack.

The upstream project reached end-of-life on September 1, 2022. This fork extends it with new protocol features, Python bindings, and modern CI.

**For commercial or production use**, consider [Step Function I/O's DNP3 library](https://stepfunc.io/products/libraries/dnp3/) — a modern, commercially supported Rust implementation with C, C++, Java, and .NET bindings.

## Features

**Protocol — Master operations:**
- Integrity / class / range / all-objects scans (periodic and on-demand)
- Select-before-operate and direct-operate commands (CROB, analog outputs)
- File transfer: read, write, delete, get info, directory listing, abort, authenticate
- Device attributes (Group 0)
- Analog input dead bands (Group 34)
- Freeze operations
- Cold/warm restart
- Time synchronization (LAN and serial modes)
- Link status check

**Protocol — Outstation:**
- Static and event reporting for all standard data types
- Unsolicited responses with configurable retries
- Command handling (select/operate, direct operate)
- File transfer handler interface
- Broadcast support
- Configurable event buffer and class assignment

**Transports:**
- TCP client and server
- Outstation TCP client mode (outstation connects to master)
- UDP
- Serial
- TLS (client and server, requires OpenSSL)

**Bindings:**
- **Python** (pybind11) — `pip install yadnp3`
- **Java** (JNI)
- **.NET** (C++/CLI, Windows)

## Python Quick Start

```bash
pip install yadnp3
```

```python
import opendnp3

# Create a manager with 1 thread
manager = opendnp3.DNP3Manager(1)

# ... create channels, masters, outstations
# See python/tests/ for full examples

manager.Shutdown()
```

## Building from Source

### Prerequisites

- CMake >= 3.11
- C++14 compiler (GCC, Clang, or MSVC 2015+)
- OpenSSL >= 1.1.1 (optional, for TLS)
- Python 3.9+ and pybind11 (optional, for Python bindings)

### Quick Build

```bash
mkdir build && cd build
cmake .. -DDNP3_TLS=ON -DDNP3_TESTS=ON
cmake --build . --parallel $(nproc)
ctest --output-on-failure
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `DNP3_TLS` | OFF | TLS support (requires OpenSSL) |
| `DNP3_TESTS` | OFF | Unit and integration tests |
| `DNP3_EXAMPLES` | OFF | Example applications |
| `DNP3_PYTHON` | OFF | Python bindings (pybind11) |
| `DNP3_JAVA` | OFF | Java JNI bindings |
| `DNP3_DOTNET` | OFF | .NET bindings (Windows) |
| `DNP3_STATIC_LIBS` | platform | Static libraries |
| `DNP3_EVERYTHING` | OFF | All optional targets |

### Python Wheel (Docker)

```bash
docker build -f Dockerfile --build-arg PYTHON_VERSION=3.12 -t yadnp3-build .
docker create --name whl yadnp3-build
docker cp whl:/wheels/. ./dist/
docker rm whl
pip install dist/*.whl
```

## Project Structure

```
yadnp3/
├── cpp/lib/              Core C++ library (~79k lines)
│   ├── include/opendnp3/ Public API headers
│   └── src/              Private implementation
├── cpp/tests/            Unit, integration, and interop tests
├── cpp/examples/         Example master/outstation applications
├── python/               Python bindings (pybind11)
├── java/                 Java bindings (JNI + Maven)
├── dotnet/               .NET bindings (C++/CLI)
├── generation/           Code generation (Scala/SBT)
├── Dockerfile            Linux wheel build
└── .github/workflows/    CI (build, test, wheel packaging)
```

## License

Licensed under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Copyright (c) 2010-2011 Green Energy Corp
Copyright (c) 2013-2022 Step Function I/O LLC
Copyright (c) 2024-2026 f0rw4rd (yadnp3 fork)
Copyright (c) 2010-2022 various contributors
