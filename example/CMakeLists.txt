cmake_minimum_required(VERSION 3.25)

project(example)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(raylib)
add_executable(example example.cpp)

target_link_libraries(example raylib)
target_compile_options(example PUBLIC -Wall -Wextra -g)