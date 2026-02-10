pybind11_add_module(opendnp3_python
    src/module.cpp
    src/enums.cpp
    src/types.cpp
    src/channel.cpp
    src/master.cpp
    src/outstation.cpp
)

set_target_properties(opendnp3_python PROPERTIES
    OUTPUT_NAME "_opendnp3"
    CXX_STANDARD 14
)

target_link_libraries(opendnp3_python PRIVATE opendnp3)

target_include_directories(opendnp3_python PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp/lib/include
    ${CMAKE_SOURCE_DIR}/cpp/lib/src
)
