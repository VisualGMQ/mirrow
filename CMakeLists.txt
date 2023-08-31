cmake_minimum_required(VERSION 3.12)
project(mirrow
    VERSION 0.1.0
    LANGUAGES CXX)


add_library(mirrow INTERFACE)
target_include_directories(mirrow INTERFACE ./include)
target_compile_features(mirrow INTERFACE cxx_std_17)

option(MIRROW_BUILD_TEST "build test" ON)

if (MIRROW_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()