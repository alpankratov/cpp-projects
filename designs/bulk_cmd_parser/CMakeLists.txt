cmake_minimum_required(VERSION 3.31.6)

set(CMAKE_C_COMPILER "/usr/bin/gcc-14")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-14")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bulk main.cpp)
add_executable(libasync.so async/main.cpp async/async.cpp)

set_target_properties(bulk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(libasync.so PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

install(TARGETS bulk RUNTIME DESTINATION bin)
install(TARGETS libasync.so RUNTIME DESTINATION bin)
