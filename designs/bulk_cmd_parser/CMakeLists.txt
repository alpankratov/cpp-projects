cmake_minimum_required(VERSION 3.31.6)

set(CMAKE_C_COMPILER "/usr/bin/gcc-14")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-14")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bulk main.cpp
        block_queue.h)

# Ensure the bulk executable is part of the default build (ALL) so it's built automatically
# when the project is built after a CMake reload.
add_custom_target(build_bulk ALL DEPENDS bulk)

set_target_properties(bulk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

install(TARGETS bulk RUNTIME DESTINATION bin)
