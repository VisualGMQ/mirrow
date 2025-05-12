macro(AddTest name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE Catch2 Mirrow::Mirrow)
    if (MIRROW_BUILD_TOMLPLUSPLUS_SERD)
        target_link_libraries(${name} PRIVATE Mirrow::Serd)
    endif()
    set_target_properties(${name} PROPERTIES FOLDER tests)
    add_test(NAME ${name}
             COMMAND $<TARGET_FILE:${name}>)
endmacro(AddTest)

add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE .)

add_subdirectory(util)
add_subdirectory(srefl)
add_subdirectory(drefl)

if (MIRROW_BUILD_TOMLPLUSPLUS_SERD)
    add_subdirectory(serd)
endif()