cmake_minimum_required(VERSION 3.20)
project(libdisassembler)

set(CMAKE_CXX_STANDARD 23)

add_library(libdisassembler STATIC
    source/spec/loader.cpp
    source/spec/spec.cpp
    source/spec/bit_pattern.cpp
    source/spec/opcode.cpp
)

target_include_directories(libdisassembler PUBLIC include)
target_link_libraries(libdisassembler PRIVATE wolv::types wolv::utils wolv::io wolv::math_eval ${NLOHMANN_JSON_LIBRARIES} ${FMT_LIBRARIES})
target_compile_options(libdisassembler PRIVATE -Wall -Wextra -Wpedantic -Werror)