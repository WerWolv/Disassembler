cmake_minimum_required(VERSION 3.20)
project(Disassembler)

set(CMAKE_CXX_STANDARD 23)

option(DISASSEMBLER_ENABLE_TESTS "Enable disassembler tests" OFF)

if (NOT USE_SYSTEM_FMT)
    if (NOT TARGET fmt::fmt)
        add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
        set(FMT_LIBRARIES fmt::fmt-header-only)
    endif ()
else()
    find_package(fmt 8.0.0 REQUIRED)
    set(FMT_LIBRARIES fmt::fmt)
endif()

if (NOT USE_SYSTEM_NLOHMANN_JSON)
    if (NOT TARGET nlohmann_json)
        add_subdirectory(external/json EXCLUDE_FROM_ALL)
        set(NLOHMANN_JSON_LIBRARIES nlohmann_json)
    endif ()
else()
    find_package(nlohmann_json 3.10.2 REQUIRED)
    set(NLOHMANN_JSON_LIBRARIES nlohmann_json::nlohmann_json)
endif ()

if (NOT TARGET wolv::types)
    add_subdirectory(external/libwolv)
endif()

add_subdirectory(lib)

if (DISASSEMBLER_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()