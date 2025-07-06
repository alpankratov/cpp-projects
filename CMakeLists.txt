cmake_minimum_required(VERSION 4.0)
project(cpp_projects)

set(CMAKE_CXX_STANDARD 20)

#add_executable(cpp_projects main.cpp)
add_executable(ip_filtering designs/ip_address_filtering.cpp)
