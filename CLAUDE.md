# OpenDNP3 - Developer Reference

## Project Overview

OpenDNP3 is the de facto reference implementation of the **DNP3 (IEEE-1815)** protocol, a standards-based SCADA (Supervisory Control and Data Acquisition) protocol used extensively in electric utility, water, and wastewater industries. The library is written in C++14 and designed for high-performance applications such as many concurrent TCP sessions or large device simulations. It also embeds with a small footprint on Linux.

- **Version**: 3.2.0
- **License**: Apache License 2.0
- **Original Authors**: Green Energy Corp, Step Function I/O LLC
- **Status**: The upstream project (github.com/dnp3/opendnp3) reached end-of-life as of September 1, 2022. This is an experimental fork.
- **C++ Lines of Code**: ~79,000 (across library, tests, bindings, and examples)

DNP3 is a layered protocol with link, transport, and application layers. OpenDNP3 implements both the **master** (polling station / control center) and **outstation** (field device) roles, supporting TCP, UDP, serial, and TLS transports.

## Architecture

### Protocol Layer Stack

The implementation follows the DNP3 protocol layering model. From bottom to top:

```
+-------------------------------------------+
|  Application (Master or Outstation)       |  MContext / OContext
+-------------------------------------------+
|  Transport Layer                          |  TransportLayer
+-------------------------------------------+
|  Link Layer                               |  LinkLayer
+-------------------------------------------+
|  Channel / Physical Layer                 |  IOHandler (TCP/UDP/Serial/TLS)
+-------------------------------------------+
```

**Layer interfaces** are defined in `cpp/lib/src/LayerInterfaces.h`:
- `IUpperLayer` - receives data from below (`OnReceive`, `OnLowerLayerUp/Down`, `OnTxReady`)
- `ILowerLayer` - sends data downward (`BeginTransmit`)
- `HasUpperLayer` / `HasLowerLayer` - mixin classes for layer wiring

The `TransportStack` class (`cpp/lib/src/transport/TransportStack.h`) bundles the transport and link layers together into a reusable unit that is shared by both master and outstation stacks via `StackBase`.

### Key Object Hierarchy

```
DNP3Manager                    Entry point, manages thread pool and channels
  -> IChannel (DNP3Channel)    Communication channel (TCP/UDP/Serial/TLS)
       -> IMaster (MasterStack)    Master session on a channel
       -> IOutstation (OutstationStack)  Outstation session on a channel
```

- **DNP3Manager** (`cpp/lib/include/opendnp3/DNP3Manager.h`): Root factory object. Creates channels and manages the ASIO thread pool.
- **IChannel** (`cpp/lib/include/opendnp3/channel/IChannel.h`): Represents a communication channel. Channels create masters and outstations.
- **IStack** (`cpp/lib/include/opendnp3/IStack.h`): Base interface for both masters and outstations (Enable/Disable/Shutdown).
- **IMaster** / **IOutstation**: Specialized stack interfaces for each role.

### Threading Model

Uses ASIO (standalone, non-Boost) with a configurable thread pool. Each channel gets a `StrandExecutor` from exe4cpp to ensure serialized access without explicit locking. The concurrency hint passed to `DNP3Manager` controls thread pool size.

## Directory Structure

```
opendnp3/
+-- CMakeLists.txt          Root build file
+-- README.md               Project overview
+-- LICENSE                  Apache 2.0
+-- CHANGELOG.md            Version history
+-- .clang-format            Code formatting rules (WebKit-based style)
+-- .clang-tidy              Static analysis configuration
+-- codecov.yml              Code coverage configuration
|
+-- cpp/                     All C++ code
|   +-- lib/                 Core library
|   |   +-- include/opendnp3/  Public API headers
|   |   |   +-- app/           Application layer types (measurements, commands, IIN)
|   |   |   +-- channel/       Channel interfaces (IChannel, serial, TLS config)
|   |   |   +-- decoder/       Protocol decoder (for diagnostic tools)
|   |   |   +-- gen/           Generated enums and variations (from code generation)
|   |   |   +-- link/          Link layer config and statistics
|   |   |   +-- logging/       Log levels, log handler interfaces
|   |   |   +-- master/        Master API (ISOEHandler, IMaster, commands, scans)
|   |   |   +-- outstation/    Outstation API (ICommandHandler, database config)
|   |   |   +-- util/          Utilities (TimeDuration, Buffer, Uncopyable)
|   |   +-- src/               Private implementation
|   |       +-- app/           APDU parsing/building, serialization, header writing
|   |       |   +-- parsing/   APDU parser, header parser, object parsers
|   |       +-- channel/       TCP/UDP/Serial/TLS I/O handlers
|   |       |   +-- tls/       TLS-specific channel implementation
|   |       +-- decoder/       Protocol decoder implementation
|   |       +-- gen/           Generated serialization code and object definitions
|   |       |   +-- objects/   DNP3 data object groups (Group1-Group113)
|   |       +-- link/          Link layer state machine, CRC, framing
|   |       +-- logging/       Console pretty printer, hex logging
|   |       +-- master/        Master state machine, tasks, scheduler
|   |       +-- outstation/    Outstation state machine, database, events
|   |       |   +-- event/     Event buffer, storage, selection, writing
|   |       +-- transport/     Transport layer reassembly/segmentation
|   |       +-- util/          TimeDuration, Timestamp implementations
|   |
|   +-- examples/              Example applications
|   |   +-- master/            TCP master example
|   |   +-- master-gprs/       GPRS master (listener mode)
|   |   +-- master-udp/        UDP master example
|   |   +-- outstation/        TCP outstation example
|   |   +-- outstation-udp/    UDP outstation example
|   |   +-- decoder/           Standalone protocol decoder
|   |   +-- tls/               TLS master and outstation examples
|   |
|   +-- tests/                 Test suites
|       +-- dnp3mocks/         Shared mock objects for tests
|       +-- unit/              Unit tests (Catch2)
|       +-- asiotests/         ASIO networking tests (TCP, TLS, strand executor)
|       +-- integration/       Integration tests (full stack, DNP3Manager)
|       +-- fuzz/              Google OSS-Fuzz targets
|
+-- java/                      Java bindings
|   +-- CMakeLists.txt         JNI shared library build
|   +-- pom.xml                Maven parent POM
|   +-- cpp/                   JNI native code (adapters, JNI wrappers)
|   |   +-- adapters/          C++ adapter classes bridging Java callbacks
|   |   +-- jni/               Auto-generated JNI method stubs
|   +-- bindings/              Java binding JAR (Maven project)
|   +-- codegen/               Java code generator (Maven project)
|   +-- example/               Java example applications
|
+-- dotnet/                    .NET bindings (Windows only)
|   +-- CMakeLists.txt         .NET build configuration
|   +-- CLRAdapter/            C++/CLI adapter layer
|   +-- CLRInterface/          .NET interface definitions
|   +-- examples/              .NET example applications
|   +-- nuget/                 NuGet package configuration
|
+-- generation/                Code generation system (Scala/SBT)
|   +-- dnp3/                  DNP3 enum/object definitions in Scala
|   +-- render/                Rendering templates for C++/Java
|
+-- deps/                      External dependency CMake files
|   +-- asio.cmake             ASIO 1.16.0 (standalone, fetched)
|   +-- exe4cpp.cmake          exe4cpp executor framework (fetched)
|   +-- ser4cpp.cmake          ser4cpp serialization library (fetched)
|   +-- catch.cmake            Catch2 v2.11.3 test framework (fetched)
|
+-- cmake/                     CMake utility modules
|   +-- ClangFormat.cmake      clang-format integration
|   +-- ClangTidy.cmake        clang-tidy integration
|   +-- CodeCoverage.cmake     lcov code coverage support
|
+-- config/                    Configuration files
|   +-- doxygen.config         Doxygen documentation generation
|   +-- APACHE_LICENSE_HEADER  License header for source files
|
+-- profile/                   DNP3 profile
|   +-- opendnp3_profile.xml   DNP3 conformance profile
|
+-- .github/workflows/         CI configuration
    +-- ci.yml                 GitHub Actions workflow
```

## Build System

### Prerequisites

- CMake >= 3.11
- C++14 compatible compiler (GCC, Clang, MSVC 2015+)
- OpenSSL >= 1.1.1 (only if building with TLS support)
- JDK + Maven (only if building Java bindings)
- .NET Framework (only if building .NET bindings on Windows)

### Basic Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel $(nproc)
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `DNP3_TLS` | OFF | Build TLS support (requires OpenSSL) |
| `DNP3_TESTS` | OFF | Build unit and integration tests |
| `DNP3_EXAMPLES` | OFF | Build example applications |
| `DNP3_FUZZING` | OFF | Build Google OSS-Fuzz targets |
| `DNP3_COVERAGE` | OFF | Enable code coverage target |
| `DNP3_JAVA` | OFF | Build Java JNI bindings |
| `DNP3_DOTNET` | OFF | Build .NET bindings (Windows only) |
| `DNP3_EVERYTHING` | OFF | Enable all optional targets |
| `DNP3_STATIC_LIBS` | ON (Win) / OFF (Linux) | Build static instead of shared libraries |

### Build Everything (Linux)

```bash
mkdir build && cd build
cmake -DDNP3_EVERYTHING=ON -DDNP3_STATIC_LIBS=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel $(nproc)
```

### Running Tests

```bash
cd build
ctest -VV
```

Test executables: `unittests`, `asiotests`, `integrationtests`

### Code Coverage

```bash
cmake -DDNP3_TESTS=ON -DDNP3_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target coverage --parallel $(nproc)
```

### Java Bindings Build

After CMake build with `DNP3_JAVA=ON`:
```bash
cd java
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/build/java
mvn --batch-mode verify
```

### Install

```bash
cmake --build . --target install
```

Installs headers to `include/`, libraries to `lib/`, and CMake config to `lib/cmake/`.

## Key Components

### Master (`cpp/lib/src/master/`)

The master implementation centers on `MContext` (`MasterContext.h`), which implements `IUpperLayer` and manages the master-side protocol state machine.

Key classes:
- **MContext**: Core state machine. Handles solicited/unsolicited responses, task scheduling, command operations.
- **MasterStack**: Combines MContext with TransportStack. Implements `IMaster`.
- **MasterSchedulerBackend**: Task scheduling engine. Manages periodic polls, one-shot operations, and task priorities.
- **MasterTasks**: Manages built-in tasks (integrity poll, event scan, time sync, unsolicited management).
- **IMasterTask**: Base class for all master tasks (polls, commands, time sync, etc.).

**Master tasks include**: `StartupIntegrityPoll`, `EventScanTask`, `UserPollTask`, `CommandTask`, `ClearRestartTask`, `EnableUnsolicitedTask`, `DisableUnsolicitedTask`, `LANTimeSyncTask`, `SerialTimeSyncTask`, `AssignClassTask`, `RestartOperationTask`.

**Callbacks**:
- `ISOEHandler` - Sequence of Events handler. Receives all measurement data from responses.
- `IMasterApplication` - Application-level callbacks (IIN processing, timestamps, task notifications).

### Outstation (`cpp/lib/src/outstation/`)

The outstation implementation centers on `OContext` (`OutstationContext.h`), which implements `IUpperLayer`.

Key classes:
- **OContext**: Core state machine handling requests, responses, unsolicited reporting, and controls.
- **OutstationStack**: Combines OContext with TransportStack. Implements `IOutstation`.
- **Database**: Stores current point values (static data). Indexed by point type and index.
- **EventBuffer** / **EventStorage**: Manages the event buffer for Class 1/2/3 event reporting.
- **ResponseContext**: Tracks multi-fragment response state for read operations.
- **OutstationState**: State pattern base class for outstation states (Idle, SolicitedConfirmWait, UnsolicitedConfirmWait, NullUnsolicitedConfirmWait).

**Callbacks**:
- `ICommandHandler` - Receives Select/Operate/DirectOperate commands from the master.
- `IOutstationApplication` - Application-level callbacks (restart, time, broadcast support).
- `IUpdateHandler` - Interface for updating the outstation database with new point values.

### Transport Layer (`cpp/lib/src/transport/`)

- **TransportLayer**: Implements both `IUpperLayer` and `ILowerLayer`, sitting between the application and link layers.
- **TransportRx**: Reassembles segmented transport frames into complete APDUs.
- **TransportTx**: Segments outgoing APDUs into transport-layer frames.
- Uses sequence numbers (FIR/FIN bits) for multi-frame message handling.

### Link Layer (`cpp/lib/src/link/`)

- **LinkLayer**: Implements `ILinkLayer` and `ILinkSession`. Manages the data link layer protocol.
- **LinkContext**: Shared state for the link layer, including CRC calculation and frame construction.
- **LinkLayerParser**: Parses incoming link-layer frames from raw bytes. Handles CRC verification.
- **PriLinkLayerStates** / **SecLinkLayerStates**: State pattern implementations for primary and secondary link station behavior.
- **LinkFrame**: Constructs outgoing link-layer frames with proper headers and CRC.
- **CRC**: DNP3 CRC-16 calculation.
- **ShiftableBuffer**: Efficient ring-buffer-like parser for incoming byte streams.

### Application Layer (`cpp/lib/src/app/`)

- **APDURequest** / **APDUResponse**: Classes for constructing and parsing DNP3 Application Protocol Data Units.
- **APDUParser** (`parsing/`): Full APDU parser for incoming messages. Dispatches parsed objects to handlers.
- **HeaderWriter**: Writes object headers and data into outgoing APDUs.
- **GroupVariationRecord**: Maps group/variation IDs to parsing logic.
- **MeasurementTypes**: Defines Binary, Analog, Counter, FrozenCounter, BinaryOutputStatus, AnalogOutputStatus, OctetString, TimeAndInterval, etc.
- **IINField**: Internal Indications field (two bytes of device status flags).

### Channel Layer (`cpp/lib/src/channel/`)

- **DNP3Channel**: Concrete channel implementation. Creates and manages master/outstation stacks on a single communication path.
- **IOHandler**: Base class for all I/O handlers. Manages connection lifecycle and session routing.
- **TCPClientIOHandler** / **TCPServerIOHandler**: TCP client and server transports.
- **SerialIOHandler**: Serial port transport.
- **UDPClientIOHandler**: UDP transport.
- **TLS** (`tls/`): TLS client and server using OpenSSL (compiled conditionally with `OPENDNP3_USE_TLS`).

### Generated Code (`cpp/lib/src/gen/`)

The `gen/` directory contains code generated by the Scala-based code generation system (`generation/`). This includes:
- **Enum types**: FunctionCode, GroupVariation, QualifierCode, CommandStatus, quality enums, variation enums, etc.
- **Serialization functions**: For each enum type.
- **Object group definitions** (`gen/objects/`): Group1 through Group113 defining the wire format for each DNP3 data object.

Do NOT manually edit files in `gen/` -- they are auto-generated.

## Code Patterns

### Coding Conventions

- **C++ Standard**: C++14 (configured via `target_compile_features(... cxx_std_14)`)
- **Namespace**: All code is in the `opendnp3` namespace
- **Formatting**: Enforced by clang-format (WebKit-derived style)
  - 4-space indentation, no tabs
  - 120 column limit
  - Allman/BSD brace style (braces on new lines)
  - Pointer alignment: left (`int* p`, not `int *p`)
  - Sorted includes with priority grouping
- **Static Analysis**: clang-tidy is configured but warnings are not treated as errors
- **Include Order**: Local private headers first, then public `opendnp3/` headers, then external (`ser4cpp`, `exe4cpp`), then system headers. Managed by `.clang-format` IncludeCategories.

### Design Patterns Used

1. **State Pattern** (used extensively):
   - Outstation: `OutstationState` with `StateIdle`, `StateSolicitedConfirmWait`, `StateUnsolicitedConfirmWait`, `StateNullUnsolicitedConfirmWait`
   - Link Layer: `PriStateBase` with `PLLS_Idle`, `PLLS_SendUnconfirmedTransmitWait`, `PLLS_RequestLinkStatusWait`; `SecStateBase` for secondary station
   - States are implemented as **singletons** via `MACRO_STATE_SINGLETON_INSTANCE` macro

2. **Singleton Pattern**: Used for state objects via macros in `Singleton.h`

3. **Factory Pattern**: `DNP3Manager::Create()`, `MContext::Create()`, `DNP3Channel::Create()`

4. **Callback/Observer Pattern**:
   - `ISOEHandler` for measurement callbacks from master
   - `ICommandHandler` for command callbacks to outstation
   - `IMasterApplication` / `IOutstationApplication` for application events
   - `IChannelListener` for channel state change notifications
   - `ILogHandler` for log output

5. **PIMPL (Pointer to Implementation)**: `DNP3Manager` uses `DNP3ManagerImpl` (private impl)

6. **Template Methods**: `StackBase::PerformShutdown<T>()`, `DNP3Channel::AddStack<T>()`

7. **Layer Pattern**: Protocol stack layers communicate through `IUpperLayer`/`ILowerLayer` interfaces

8. **Uncopyable**: Base class to prevent copy construction/assignment (used pervasively)

9. **shared_ptr Ownership**: Resources are managed via `std::shared_ptr` with `enable_shared_from_this` for safe self-referencing. `ResourceManager` tracks all created resources for orderly shutdown.

### Naming Conventions

- **Classes**: PascalCase (`MasterContext`, `TransportLayer`)
- **Methods**: PascalCase (`OnReceive`, `BeginTransmit`)
- **Member variables**: camelCase (`isOnline`, `responseTimer`)
- **Constants**: UPPER_CASE or PascalCase
- **Files**: PascalCase matching class name (`MasterContext.cpp`)
- **Include guards**: `OPENDNP3_CLASSNAME_H`

## Testing

### Test Framework

Uses **Catch2** v2.11.3 (header-only, auto-fetched by CMake).

### Test Structure

**Unit Tests** (`cpp/tests/unit/`):
Tests for each protocol layer and component. Test files named `Test<Component>.cpp`. Tests use fixtures from `utils/` directory.
- `TestLinkLayer.cpp`, `TestLinkReceiver.cpp`, `TestLinkFrame.cpp` - Link layer tests
- `TestTransportLayer.cpp` - Transport layer tests
- `TestMaster.cpp`, `TestMasterCommandRequests.cpp`, `TestMasterUnsolBehaviors.cpp` - Master tests
- `TestOutstation.cpp`, `TestOutstationEventResponses.cpp`, `TestOutstationCommandResponses.cpp` - Outstation tests
- `TestAPDUParsing.cpp`, `TestAPDUWriting.cpp` - Application layer parsing/building tests
- `TestDatabase.cpp`, `TestEventStorage.cpp` - Data model tests

**Test Utilities** (`cpp/tests/unit/utils/`):
- `MasterTestFixture` - Sets up a master with mock lower layer for isolated testing
- `OutstationTestObject` - Sets up an outstation with mock lower layer
- `LinkLayerTest` - Link layer test harness
- `TransportTestObject` - Transport layer test harness

**Mock Objects** (`cpp/tests/dnp3mocks/`):
Reusable mock implementations for all major interfaces:
- `MockSOEHandler`, `MockCommandHandler`, `MockMasterApplication`, `MockOutstationApplication`
- `MockLinkLayer`, `MockLowerLayer`, `MockUpperLayer`, `MockTransportLayer`
- `MockLogHandler`, `MockFrameSink`, `MockLinkListener`

**ASIO Tests** (`cpp/tests/asiotests/`):
Tests for networking components (TCP client/server, strand executor, TLS).

**Integration Tests** (`cpp/tests/integration/`):
Full-stack tests exercising `DNP3Manager` with real TCP connections:
- `TestDNP3Manager.cpp` - Manager lifecycle tests
- `TestEventIntegration.cpp` - End-to-end event reporting
- `TestMasterServerSmoke.cpp` - Master server acceptance tests
- `TestPerformance.cpp` - Performance benchmarks
- `TestDeadlock.cpp` - Concurrency regression tests

**Fuzz Tests** (`cpp/tests/fuzz/`):
Google OSS-Fuzz targets:
- `fuzzmaster.cpp` - Fuzz the master parser
- `fuzzoutstation.cpp` - Fuzz the outstation parser
- `fuzzdecoder.cpp` - Fuzz the protocol decoder

### Running Tests

```bash
# Build with tests
cmake -DDNP3_TESTS=ON ..
cmake --build . --parallel $(nproc)

# Run all tests
ctest -VV

# Run specific test executable
./unittests
./asiotests
./integrationtests
```

## Language Bindings

### Java Bindings (`java/`)

Full Java bindings via JNI. The native library (`opendnp3java`) is built as a shared library by CMake. The Java side is managed by Maven.

**Structure**:
- `java/cpp/` - JNI native code with adapters bridging Java interfaces to C++ callbacks
- `java/cpp/adapters/` - `SOEHandlerAdapter`, `CommandHandlerAdapter`, `MasterApplicationAdapter`, etc.
- `java/cpp/jni/` - Auto-generated JNI wrapper classes for each Java type
- `java/bindings/` - Maven project producing the Java binding JAR
- `java/codegen/` - Maven project for generating JNI code
- `java/example/` - Java example applications

The Java package namespace is `com.automatak.dnp3`.

Maven coordinates: defined in `java/pom.xml` (group: `com.automatak.dnp3`)

### .NET Bindings (`dotnet/`)

Windows-only .NET bindings via C++/CLI.

- `dotnet/CLRInterface/` - .NET interface definitions (pure managed code)
- `dotnet/CLRAdapter/` - C++/CLI bridge between .NET and native C++
- `dotnet/examples/` - Example .NET applications (master, master-gprs, outstation)
- `dotnet/nuget/` - NuGet package generation configuration

## Dependencies

All external dependencies are fetched automatically by CMake (FetchContent):

| Dependency | Version | Purpose |
|-----------|---------|---------|
| **ASIO** | 1.16.0 | Standalone (non-Boost) async I/O for networking and timers |
| **exe4cpp** | (pinned commit) | Executor/thread pool framework (from Automatak) |
| **ser4cpp** | (pinned commit) | Serialization library with buffer types (from Automatak) |
| **Catch2** | 2.11.3 | C++ test framework (header-only) |
| **OpenSSL** | >= 1.1.1 | TLS support (optional, system-installed) |
| **JNI/JDK** | any | Java bindings (optional) |

## Important Files

### Entry Points and Core API

| File | Purpose |
|------|---------|
| `cpp/lib/include/opendnp3/DNP3Manager.h` | **Main entry point** - creates channels, manages lifetime |
| `cpp/lib/include/opendnp3/channel/IChannel.h` | Channel interface - creates masters/outstations |
| `cpp/lib/include/opendnp3/IStack.h` | Base interface for master/outstation stacks |
| `cpp/lib/include/opendnp3/master/IMaster.h` | Master-specific operations |
| `cpp/lib/include/opendnp3/outstation/IOutstation.h` | Outstation-specific operations |

### Key Callback Interfaces

| File | Purpose |
|------|---------|
| `cpp/lib/include/opendnp3/master/ISOEHandler.h` | Receives parsed measurements from master |
| `cpp/lib/include/opendnp3/outstation/ICommandHandler.h` | Handles Select/Operate commands at outstation |
| `cpp/lib/include/opendnp3/master/IMasterApplication.h` | Master application callbacks |
| `cpp/lib/include/opendnp3/outstation/IOutstationApplication.h` | Outstation application callbacks |
| `cpp/lib/include/opendnp3/channel/IChannelListener.h` | Channel state change notifications |
| `cpp/lib/include/opendnp3/logging/ILogHandler.h` | Custom log handler interface |

### Configuration

| File | Purpose |
|------|---------|
| `cpp/lib/include/opendnp3/master/MasterParams.h` | Master configuration (timeouts, polling, sync) |
| `cpp/lib/include/opendnp3/master/MasterStackConfig.h` | Full master stack config (includes link) |
| `cpp/lib/include/opendnp3/outstation/OutstationParams.h` | Outstation parameters |
| `cpp/lib/include/opendnp3/outstation/OutstationStackConfig.h` | Full outstation stack config |
| `cpp/lib/include/opendnp3/outstation/DatabaseConfig.h` | Outstation database point configuration |
| `cpp/lib/include/opendnp3/link/LinkConfig.h` | Link layer configuration (addresses, timeouts) |
| `cpp/lib/include/opendnp3/channel/ChannelRetry.h` | Channel reconnect retry parameters |

### Implementation Core

| File | Purpose |
|------|---------|
| `cpp/lib/src/DNP3ManagerImpl.h` | Manager private implementation |
| `cpp/lib/src/channel/DNP3Channel.h` | Channel implementation |
| `cpp/lib/src/master/MasterContext.h` | Master state machine core |
| `cpp/lib/src/outstation/OutstationContext.h` | Outstation state machine core |
| `cpp/lib/src/transport/TransportLayer.h` | Transport layer |
| `cpp/lib/src/link/LinkLayer.h` | Link layer |
| `cpp/lib/src/LayerInterfaces.h` | IUpperLayer/ILowerLayer interfaces |
| `cpp/lib/src/StackBase.h` | Shared base for master/outstation stacks |
| `cpp/lib/src/transport/TransportStack.h` | Bundled transport+link layers |

### Build Configuration

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Root build file with all options |
| `cpp/lib/CMakeLists.txt` | Library build (lists all source files) |
| `deps/*.cmake` | External dependency fetch scripts |
| `.github/workflows/ci.yml` | GitHub Actions CI pipeline |
| `.clang-format` | Code formatting rules |
| `.clang-tidy` | Static analysis rules |

## CI Pipeline

GitHub Actions workflow (`.github/workflows/ci.yml`) runs on push and pull requests:

1. **Linux CI** (matrix: gcc/clang x Debug/Release):
   - Full build with `DNP3_EVERYTHING=ON`
   - Run C++ tests via `ctest`
   - Package and upload artifacts
   - Build and test Java bindings via Maven

2. **Windows CI** (matrix: Debug/Release x x86/x64):
   - Full build with `DNP3_EVERYTHING=ON`
   - Run C++ tests
   - Build .NET bindings
   - Package NuGet artifacts
   - Build and test Java bindings

3. **Code Coverage** (Linux, gcc, Debug):
   - Build with `DNP3_COVERAGE=ON`
   - Run tests and upload to Codecov

4. **NuGet Package** (depends on Windows CI):
   - Aggregates x86/x64 builds into a single NuGet package
   - Publishes on tagged releases

5. **Conformance Testing** (depends on Linux CI):
   - Runs against dnp4s conformance test suite
   - Uploads test results as artifacts

## Logging

The logging system uses bitfield-based log levels defined in `cpp/lib/include/opendnp3/logging/LogLevels.h`:

- `flags::EVENT`, `flags::ERR`, `flags::WARN`, `flags::INFO`, `flags::DBG` - standard levels
- `flags::LINK_RX`, `flags::LINK_TX` - link layer I/O
- `flags::TRANSPORT_RX`, `flags::TRANSPORT_TX` - transport layer I/O
- `flags::APP_HEADER_RX/TX`, `flags::APP_OBJECT_RX/TX`, `flags::APP_HEX_RX/TX` - application layer

Predefined combinations:
- `levels::NORMAL` = EVENT | ERR | WARN | INFO
- `levels::ALL_APP_COMMS` = all application-layer communication logging
- `levels::ALL_COMMS` = all communication logging across all layers

Implement `ILogHandler` to receive log output. `ConsoleLogger` is provided as a default.

## Git and Version Control

- **Main branch**: `release` (this is the default and only remote-tracking branch)
- **Never commit as AI** -- keep author attribution to actual contributors
- **Use minimal commit messages** -- short and descriptive
- **Upstream branching** (historical): `release-2.x`, `develop` branches existed in the upstream repo
- **Maven release plugin** is used for Java artifact versioning

## Code Generation

The `generation/` directory contains a Scala-based code generation system that produces:
- C++ enum definitions and serialization code (`cpp/lib/src/gen/`, `cpp/lib/include/opendnp3/gen/`)
- Java enum and type definitions
- DNP3 object group wire format definitions

The generator is built with Maven (`generation/pom.xml`). Source is in `generation/dnp3/src/main/scala/com/automatak/render/dnp3/`.

If you need to add a new DNP3 object group, variation, or enum value, modify the Scala definitions and regenerate rather than editing generated files directly.
