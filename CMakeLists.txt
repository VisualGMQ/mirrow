cmake_minimum_required(VERSION 3.20)
project(mirrow
    VERSION 1.0.0
    LANGUAGES C CXX)

option(MIRROW_BUILD_TOMLPLUSPLUS_SERD "build serialization of toml++" OFF)
option(MIRROW_BUILD_TEST "build test" OFF)

if (PROJECT_IS_TOP_LEVEL)
    set(MIRROW_BUILD_TOMLPLUSPLUS_SERD ON)
    set(MIRROW_BUILD_TEST ON)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(mirrow)

if (MIRROW_BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()