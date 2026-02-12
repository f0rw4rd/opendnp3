set(interoptests_headers
    ./InteropFixture.h
)

set(interoptests_src
    ./main.cpp

    ./TestAnalogOutputStatus.cpp
    ./TestBasicConnectivity.cpp
    ./TestBinaryOutputStatus.cpp
    ./TestCommands.cpp
    ./TestDeviceAttributes.cpp
    ./TestDoubleBitBinary.cpp
    ./TestFileTransfer.cpp
    ./TestFrozenAnalogAndCounters.cpp
    ./TestOctetString.cpp
    ./TestTLS.cpp
    ./TestWriteDeadBands.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${interoptests_headers} ${interoptests_src}
)

add_executable(interoptests
    ${interoptests_headers} ${interoptests_src}
)
target_compile_features(interoptests PRIVATE cxx_std_14)
target_link_libraries(interoptests PRIVATE catch opendnp3 dnp3mocks stepfunc_dnp3)
target_include_directories(interoptests PRIVATE ./)
set_target_properties(interoptests PROPERTIES FOLDER cpp/tests)
add_test(NAME interoptests COMMAND interoptests)

if(DNP3_TLS)
    target_link_libraries(interoptests PRIVATE OpenSSL::SSL)
    target_compile_definitions(interoptests PRIVATE OPENDNP3_USE_TLS)

    set(INTEROP_CERT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/certs")
    set(INTEROP_CERT_DST "${CMAKE_CURRENT_BINARY_DIR}/certs")
    configure_file(${INTEROP_CERT_SRC}/self_signed/entity1_cert.pem ${INTEROP_CERT_DST}/self_signed/entity1_cert.pem COPYONLY)
    configure_file(${INTEROP_CERT_SRC}/self_signed/entity1_key.pem ${INTEROP_CERT_DST}/self_signed/entity1_key.pem COPYONLY)
    configure_file(${INTEROP_CERT_SRC}/self_signed/entity2_cert.pem ${INTEROP_CERT_DST}/self_signed/entity2_cert.pem COPYONLY)
    configure_file(${INTEROP_CERT_SRC}/self_signed/entity2_key.pem ${INTEROP_CERT_DST}/self_signed/entity2_key.pem COPYONLY)
    target_compile_definitions(interoptests PRIVATE INTEROP_CERT_DIR="${INTEROP_CERT_DST}")
endif()
