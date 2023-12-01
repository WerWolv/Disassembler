cmake_minimum_required(VERSION 3.15)
project(disassembler_tests)

set(CMAKE_CXX_STANDARD 23)

add_executable(disassembler_tests
    source/main.cpp
)

target_include_directories(disassembler_tests PUBLIC include)
target_link_libraries(disassembler_tests PUBLIC libdisassembler)
