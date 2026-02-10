include(FetchContent)

FetchContent_Declare(
    pybind11
    URL      https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.zip
    URL_HASH SHA256=d0a116e91f64a4a2d8fb7590c34242df92258a61ec644b79127951e821b47be6
)

FetchContent_GetProperties(pybind11)
if(NOT pybind11_POPULATED)
    FetchContent_Populate(pybind11)
    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
endif()
