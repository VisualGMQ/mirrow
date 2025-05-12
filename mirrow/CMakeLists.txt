file(GLOB_RECURSE MIRROW_SRC 
    ./mirrow/drefl/*.hpp ./mirrow/srefl/*.hpp
    ./src/drefl/*.cpp)

file(GLOB_RECURSE MIRROW_UTIL_SRC ./mirrow/util/*.hpp)
file(GLOB_RECURSE MIRROW_DREFL_SRC ./mirrow/drefl/*.hpp ./src/drefl/*.cpp)
file(GLOB_RECURSE MIRROW_SREFL_SRC ./mirrow/srefl/*.hpp ./src/srefl/*.cpp)

add_library(Mirrow_Util INTERFACE ${MIRROW_UTIL_SRC})
target_include_directories(Mirrow_Util INTERFACE .)
target_compile_features(Mirrow_Util INTERFACE cxx_std_17)
add_library(Mirrow::Util ALIAS Mirrow_Util)

add_library(Mirrow_Srefl INTERFACE ${MIRROW_SREFL_SRC})
target_include_directories(Mirrow_Srefl INTERFACE .)
target_link_libraries(Mirrow_Srefl INTERFACE Mirrow_Util)
target_compile_features(Mirrow_Srefl INTERFACE cxx_std_17)
add_library(Mirrow::Srefl ALIAS Mirrow_Srefl)

add_library(Mirrow_Drefl STATIC ${MIRROW_DREFL_SRC})
target_include_directories(Mirrow_Drefl PUBLIC .)
target_link_libraries(Mirrow_Drefl PUBLIC Mirrow_Util)
target_compile_features(Mirrow_Drefl PUBLIC cxx_std_17)
add_library(Mirrow::Drefl ALIAS Mirrow_Drefl)

add_library(Mirrow_Mirrow INTERFACE)
target_link_libraries(Mirrow_Mirrow INTERFACE Mirrow_Util Mirrow_Drefl Mirrow_Srefl)
add_library(Mirrow::Mirrow ALIAS Mirrow_Mirrow)

if (MIRROW_BUILD_TOMLPLUSPLUS_SERD)
    add_subdirectory(serd_backend/tomlplusplus)
    file(GLOB_RECURSE MIRROW_SERD_SRC ./mirrow/serd/*.hpp ./src/serd/*.cpp)
    add_library(Mirrow_Serd STATIC ${MIRROW_SERD_SRC})
    target_include_directories(Mirrow_Serd PUBLIC .)
    target_link_libraries(Mirrow_Serd PUBLIC Mirrow_Mirrow tomlplusplus::tomlplusplus)
    add_library(Mirrow::Serd ALIAS Mirrow_Serd)
endif()