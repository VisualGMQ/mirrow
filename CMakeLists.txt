cmake_minimum_required(VERSION 3.12)
project(mirrow
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(src/drefl mirrow_src)
aux_source_directory(src/serd/dynamic/backends mirrow_src)
add_library(mirrow STATIC ${mirrow_src})
target_include_directories(mirrow PUBLIC ./include)
target_compile_features(mirrow PUBLIC cxx_std_17)

option(MIRROW_BUILD_TEST "build test" OFF)
option(MIRROW_TEST_ENABLE_TOML++ "enable test serialization with toml++" OFF)

if (PROJECT_IS_TOP_LEVEL)
    set(MIRROW_BUILD_TEST ON CACHE BOOL "build test" FORCE)
    set(MIRROW_TEST_ENABLE_TOML++ ON CACHE BOOL "enable test serialization with toml++" FORCE)
endif()

if (MIRROW_BUILD_TEST)
    enable_testing()
    if (MIRROW_TEST_ENABLE_TOML++)
        add_subdirectory(serd_backend/tomlplusplus)
        target_link_libraries(mirrow PRIVATE tomlplusplus::tomlplusplus)
    endif()
    add_subdirectory(test)
endif()