cmake_minimum_required(VERSION 3.12)
project(mirrow
    VERSION 0.1.0
    LANGUAGES CXX)


add_library(mirrow INTERFACE)
target_include_directories(mirrow INTERFACE ./include)
target_compile_features(mirrow INTERFACE cxx_std_17)

if (PROJECT_IS_TOP_LEVEL)
    option(MIRROW_BUILD_TEST "build test" ON)
    option(MIRROW_TEST_ENABLE_TOML++ "enable test serialization with toml++" ON)

    if (MIRROW_BUILD_TEST)
        enable_testing()
        if (MIRROW_TEST_ENABLE_TOML++)
            add_subdirectory(serd_backend/tomlplusplus)
            target_link_libraries(mirrow INTERFACE tomlplusplus::tomlplusplus)
        endif()
        add_subdirectory(test)
    endif()
endif()