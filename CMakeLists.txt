cmake_minimum_required(VERSION 3.15)
project(Disassembler)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(external/json)
add_subdirectory(external/libwolv)

add_subdirectory(lib)

enable_testing()
add_subdirectory(tests)
