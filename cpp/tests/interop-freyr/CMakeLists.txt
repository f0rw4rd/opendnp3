# Interoperability tests: opendnp3 vs FreyrSCADA DNP3.
#
# The FreyrSCADA DNP3 library is proprietary software.
# It is NOT redistributed with opendnp3 -- the Docker build
# downloads it at build time solely for interoperability testing.

set(freyr_interop_headers
    ./FreyrWrapper.h
)

set(freyr_interop_src
    ./main.cpp
    ./TestOpendnp3MasterFreyrOutstation.cpp
    ./TestFreyrMasterOpendnp3Outstation.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${freyr_interop_headers} ${freyr_interop_src}
)

add_executable(freyr-interoptests
    ${freyr_interop_headers} ${freyr_interop_src}
)

target_compile_features(freyr-interoptests PRIVATE cxx_std_14)

target_link_libraries(freyr-interoptests PRIVATE
    catch
    opendnp3
    dnp3mocks
    freyrscada_dnp3
)

target_include_directories(freyr-interoptests PRIVATE ./)

set_target_properties(freyr-interoptests PROPERTIES FOLDER cpp/tests)

add_test(NAME freyr-interoptests COMMAND freyr-interoptests)
