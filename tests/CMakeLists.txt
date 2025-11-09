cmake_minimum_required(VERSION 3.25)

project(tests)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

include_directories(../include)

add_executable(allocators allocators.cpp)
add_executable(ecs        ecs.cpp)
add_executable(math       math.cpp)
add_executable(test       test.cpp)
add_executable(types      types.cpp)
add_executable(lua        lua.cpp)

target_link_libraries(lua -llua5.4) # TODO FindLua

add_compile_options(PUBLIC -g
    -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy 
    -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op 
    -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual 
    -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel 
    -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros 
    -Wno-parentheses -fdiagnostics-show-option
)