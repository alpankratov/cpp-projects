cmake_minimum_required(VERSION 3.14)
project(bayan LANGUAGES CXX)

# Require C++17 (or later) – Boost works fine with it.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Find Boost components we need.
find_package(Boost 1.71 REQUIRED COMPONENTS
        filesystem
        program_options
        regex)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif ()

message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# --------------------------------------------------------------
# Library target (optional – makes unit‑testing easier)
# --------------------------------------------------------------
add_library(bayan_lib
        src/config.cpp
        src/block_reader.cpp
        src/duplicate_finder.cpp
        src/hasher_factory.cpp
)

target_include_directories(bayan_lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(bayan_lib
        PUBLIC
        Boost::filesystem
        Boost::program_options
        Boost::regex
)

# --------------------------------------------------------------
# Executable
# --------------------------------------------------------------
add_executable(bayan src/main.cpp)
target_link_libraries(bayan PRIVATE bayan_lib)
# Emit the bayan executable into the root build directory
set_target_properties(bayan PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# --------------------------------------------------------------
# Install rules (so the binary can be published on Bintray)
# --------------------------------------------------------------
install(TARGETS bayan RUNTIME DESTINATION bin)
install(TARGETS bayan_lib
        EXPORT bayan_libTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include)
install(DIRECTORY include/ DESTINATION include)

# Optional: create a basic package config for downstream projects
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/bayan-config-version.cmake"
        VERSION 1.0.0
        COMPATIBILITY SameMajorVersion)

export(TARGETS bayan_lib NAMESPACE bayan:: FILE "${CMAKE_CURRENT_BINARY_DIR}/bayan-lib-targets.cmake")
#configure_file(cmake/bayan-config.cmake.in
#        "${CMAKE_CURRENT_BINARY_DIR}/bayan-config.cmake" @ONLY)

install(EXPORT bayan_libTargets
        FILE bayan-lib-targets.cmake
        NAMESPACE bayan::
        DESTINATION lib/cmake/bayan)
