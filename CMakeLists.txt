cmake_minimum_required(VERSION 3.31.6)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(cpp_projects_lib VERSION ${PROJECT_VESRION})

option(WITH_BOOST_TEST "Whether to build Boost test" ON)

configure_file(version.h.in version.h)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp_projects main.cpp)
add_library(cpp_projects_lib lib.cpp)
add_executable(ip_filtering designs/ip_filter/ip_filter.cpp)
add_executable(ip_address_templates designs/ip_address_templates/ip_address_templates.cpp)

target_include_directories(cpp_projects_lib
        PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(cpp_projects PRIVATE
        cpp_projects_lib
)

if(WITH_BOOST_TEST)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
    add_executable(test_version test_version.cpp)

    set_target_properties(test_version PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
    )

    target_include_directories(test_version PRIVATE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(test_version PRIVATE BOOST_TEST_DYN_LINK)

    target_link_libraries(test_version
            ${Boost_LIBRARIES}
            cpp_projects_lib
    )
endif()

if (MSVC)
    target_compile_options(cpp_projects PRIVATE
            /W4
    )
    target_compile_options(cpp_projects_lib PRIVATE
            /W4
    )
    if(WITH_BOOST_TEST)
        target_compile_options(cpp_projects PRIVATE
                /W4
        )
    endif()
else ()
    target_compile_options(cpp_projects PRIVATE
            -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(cpp_projects_lib PRIVATE
            -Wall -Wextra -pedantic -Werror
    )
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE
                -Wall -Wextra -pedantic -Werror
        )
    endif()
endif()

install(TARGETS cpp_projects RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT alex@pank.me)
include(CPack)

if(WITH_BOOST_TEST)
    enable_testing()
    add_test(test_version test_version)
endif()