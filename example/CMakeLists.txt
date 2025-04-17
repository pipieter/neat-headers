cmake_minimum_required(VERSION 3.25)

project(examples)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(raylib)
add_executable(allocators_binary_tree allocators_binary_tree.cpp)
add_executable(ecs_transform          ecs_transform.cpp)

target_link_libraries(ecs_transform raylib)
target_compile_options(ecs_transform PUBLIC -g
    -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy 
    -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op 
    -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual 
    -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel 
    -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros 
    -Wno-parentheses -fdiagnostics-show-option
)