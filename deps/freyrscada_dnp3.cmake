# Find the FreyrSCADA DNP3 static library.
#
# The FreyrSCADA DNP3 library is proprietary software owned by
# FreyrSCADA Embedded Solution Pvt Ltd.  It is used here solely
# for interoperability testing and is NOT redistributed.
#
# Expects FREYRSCADA_DNP3_ROOT to point to the extracted SDK
# directory, e.g. /opt/freyr-dnp3/LinuxSDK/x86_64
#
# Creates an IMPORTED target: freyrscada_dnp3

if(NOT DEFINED FREYRSCADA_DNP3_ROOT)
    # Check common locations
    if(EXISTS "/opt/freyr-dnp3/LinuxSDK/x86_64")
        set(FREYRSCADA_DNP3_ROOT "/opt/freyr-dnp3/LinuxSDK/x86_64")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/external/freyr-dnp3/LinuxSDK/x86_64")
        set(FREYRSCADA_DNP3_ROOT "${CMAKE_SOURCE_DIR}/external/freyr-dnp3/LinuxSDK/x86_64")
    else()
        message(FATAL_ERROR
            "FREYRSCADA_DNP3_ROOT not set and no SDK found at standard locations.\n"
            "Set -DFREYRSCADA_DNP3_ROOT=/path/to/LinuxSDK/x86_64"
        )
    endif()
endif()

message(STATUS "FreyrSCADA DNP3 SDK root: ${FREYRSCADA_DNP3_ROOT}")

# Verify essential files exist
set(_freyr_lib "${FREYRSCADA_DNP3_ROOT}/library/libx86_x64-dnp3.a")
set(_freyr_inc "${FREYRSCADA_DNP3_ROOT}/header")

if(NOT EXISTS "${_freyr_lib}")
    message(FATAL_ERROR "FreyrSCADA static library not found: ${_freyr_lib}")
endif()
if(NOT EXISTS "${_freyr_inc}/dnp3api.h")
    message(FATAL_ERROR "FreyrSCADA headers not found in: ${_freyr_inc}")
endif()

# Create imported target
add_library(freyrscada_dnp3 STATIC IMPORTED GLOBAL)
set_target_properties(freyrscada_dnp3 PROPERTIES
    IMPORTED_LOCATION "${_freyr_lib}"
    INTERFACE_INCLUDE_DIRECTORIES "${_freyr_inc}"
)

# FreyrSCADA requires pthreads and rt
find_package(Threads REQUIRED)
set_property(TARGET freyrscada_dnp3 APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES Threads::Threads rt
)
