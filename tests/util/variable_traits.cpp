#include "mirrow/util/variable_traits.hpp"
#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mirrow::util;

float* gVar;

struct Clazz {
    float var;
    static float svar;
    const float cvar;
};

TEST_CASE("varibale pointer traits") {
    SECTION("global variable") {
        using traits = variable_pointer_traits<&gVar>;
        static_assert(!traits::is_member);
        static_assert(std::is_same_v<traits::pointer, float**>);
        static_assert(std::is_same_v<traits::type, float*>);
    }

    SECTION("static class variable") {
        using traits = variable_pointer_traits<&Clazz::svar>;
        static_assert(!traits::is_member);
        static_assert(std::is_same_v<traits::pointer, float*>);
        static_assert(std::is_same_v<traits::type, float>);
    }

    SECTION("const class variable") {
        using traits = variable_pointer_traits<&Clazz::cvar>;
        static_assert(traits::is_member);
        static_assert(std::is_same_v<traits::pointer, const float(Clazz::*)>);
        static_assert(std::is_same_v<traits::type, const float>);
    }
}
