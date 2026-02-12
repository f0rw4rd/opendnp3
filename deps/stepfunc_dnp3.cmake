include(FetchContent)

set(STEPFUNC_DNP3_VERSION "1.7.0-RC1")
set(STEPFUNC_DNP3_URL "https://github.com/stepfunc/dnp3/releases/download/${STEPFUNC_DNP3_VERSION}/dnp3-${STEPFUNC_DNP3_VERSION}.zip")

FetchContent_Declare(
    stepfunc_dnp3
    URL ${STEPFUNC_DNP3_URL}
)

FetchContent_GetProperties(stepfunc_dnp3)
if(NOT stepfunc_dnp3_POPULATED)
    FetchContent_Populate(stepfunc_dnp3)

    set(STEPFUNC_DNP3_PREFIX "${stepfunc_dnp3_SOURCE_DIR}")

    # Determine the Rust target triple for the current platform
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set(STEPFUNC_RUST_TARGET "x86_64-unknown-linux-gnu")
            set(STEPFUNC_LIB_NAME "libdnp3_ffi.so")
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(STEPFUNC_RUST_TARGET "x86_64-pc-windows-msvc")
            set(STEPFUNC_LIB_NAME "dnp3_ffi.dll")
            set(STEPFUNC_IMPLIB_NAME "dnp3_ffi.dll.lib")
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set(STEPFUNC_RUST_TARGET "aarch64-unknown-linux-gnu")
            set(STEPFUNC_LIB_NAME "libdnp3_ffi.so")
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set(STEPFUNC_RUST_TARGET "armv7-unknown-linux-gnueabihf")
            set(STEPFUNC_LIB_NAME "libdnp3_ffi.so")
        endif()
    endif()

    if(NOT STEPFUNC_RUST_TARGET)
        message(FATAL_ERROR "stepfunc/dnp3: unsupported platform ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(STEPFUNC_LIB_DIR "${STEPFUNC_DNP3_PREFIX}/lib/${STEPFUNC_RUST_TARGET}")
    set(STEPFUNC_LIB_PATH "${STEPFUNC_LIB_DIR}/${STEPFUNC_LIB_NAME}")

    if(NOT EXISTS "${STEPFUNC_LIB_PATH}")
        message(FATAL_ERROR "stepfunc/dnp3: prebuilt library not found at ${STEPFUNC_LIB_PATH}")
    endif()

    message(STATUS "stepfunc/dnp3: using prebuilt ${STEPFUNC_RUST_TARGET} library from ${STEPFUNC_LIB_PATH}")

    # Create the imported shared library target for the FFI layer
    add_library(stepfunc_dnp3_ffi SHARED IMPORTED GLOBAL)
    set_target_properties(stepfunc_dnp3_ffi PROPERTIES
        IMPORTED_LOCATION "${STEPFUNC_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${STEPFUNC_DNP3_PREFIX}/include"
    )
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_target_properties(stepfunc_dnp3_ffi PROPERTIES
            IMPORTED_NO_SONAME TRUE
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_target_properties(stepfunc_dnp3_ffi PROPERTIES
            IMPORTED_IMPLIB "${STEPFUNC_LIB_DIR}/${STEPFUNC_IMPLIB_NAME}"
        )
    endif()

    # Create the C++ wrapper static library
    # The dnp3.cpp file implements the C++ classes that call into the C FFI
    add_library(stepfunc_dnp3_cpp STATIC "${STEPFUNC_DNP3_PREFIX}/src/dnp3.cpp")
    target_compile_features(stepfunc_dnp3_cpp PUBLIC cxx_std_14)
    target_link_libraries(stepfunc_dnp3_cpp PUBLIC stepfunc_dnp3_ffi)

    # Convenience target that bundles both
    add_library(stepfunc_dnp3 INTERFACE)
    target_link_libraries(stepfunc_dnp3 INTERFACE stepfunc_dnp3_cpp)
endif()
