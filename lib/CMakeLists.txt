cmake_minimum_required(VERSION 3.15)
project(libdisassembler)

set(CMAKE_CXX_STANDARD 23)

add_library(libdisassembler INTERFACE

)

target_include_directories(libdisassembler INTERFACE include)
