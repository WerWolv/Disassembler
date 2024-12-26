cmake_minimum_required(VERSION 3.20)
project(Disassembler)

set(CMAKE_CXX_STANDARD 23)

option(DISASSEMBLER_ENABLE_TESTS "Enable disassembler tests" OFF)

if (NOT TARGET nlohmann_json::nlohmann_json)
    add_subdirectory(external/json)
endif()

if (NOT TARGET wolv::types)
    add_subdirectory(external/libwolv)
endif()

if (NOT TARGET fmt::fmt)
    add_subdirectory(external/fmt)
endif()

add_subdirectory(lib)

if (DISASSEMBLER_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()