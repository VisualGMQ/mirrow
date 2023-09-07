#include "mirrow/util/const_str.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("const string") {
    constexpr auto str = CONST_STR("hello");
    REQUIRE(str.str() == "hello");
}