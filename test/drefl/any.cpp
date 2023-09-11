#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/drefl.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>

int gCopyCtorCount = 0;
int gCopyAssignCount = 0;
int gMoveCtorCount = 0;

struct Foo {
    Foo(int value) : value(value) {}

    Foo() = default;

    Foo(const Foo& o): value(o.value) {
        gCopyCtorCount ++;
    }

    Foo(Foo&& o) {
        value = o.value;
        gMoveCtorCount ++;
    }

    Foo& operator=(const Foo& o) {
        value = o.value; 
        gCopyAssignCount ++;
        return *this;
    }

    int value = 0;
};

TEST_CASE("any") {
    mirrow::drefl::any a = Foo{123};
    REQUIRE(gCopyCtorCount == 1);
    REQUIRE(gCopyAssignCount == 0);
    REQUIRE(gMoveCtorCount == 0);
    REQUIRE(a.type_info() == mirrow::drefl::reflected_type<Foo>().type_node());
    REQUIRE(a.has_value());
    REQUIRE(a.try_cast<Foo>()->value == 123);
    REQUIRE(a.try_cast<const Foo&>()->value == 123);
    REQUIRE(a.can_cast<Foo>());
    REQUIRE(a.can_cast<Foo&>());
    REQUIRE(a.can_cast<const Foo&>());
    REQUIRE(!a.can_cast<int>());

    mirrow::drefl::any b;
    b = a;
    REQUIRE(gCopyCtorCount == 2);
    REQUIRE(gCopyAssignCount == 0);
    REQUIRE(gMoveCtorCount == 0);
    REQUIRE(b.has_value());
    REQUIRE(a.has_value());
    REQUIRE(b.try_cast<Foo>()->value == 123);

    mirrow::drefl::any c = std::move(b);
    REQUIRE(gCopyCtorCount == 2);
    REQUIRE(gCopyAssignCount == 0);
    REQUIRE(gMoveCtorCount == 0);
    REQUIRE(!b.has_value());
    REQUIRE(c.has_value());
    REQUIRE(c.try_cast<Foo>()->value == 123);
}

int inc(int value) {
    return value + 1;
}

Foo& clear_value(Foo& f) {
    f.value = 0;
    return f;
}

Foo& sum_value(Foo& f1, const Foo& f2) {
    f1.value = f1.value + f2.value;
    return f1;
}

TEST_CASE("invoke") {
    SECTION("simple parameters") {
        mirrow::drefl::any a = 123;

        auto result = mirrow::drefl::internal::invoke<inc>(&a, std::make_index_sequence<1>());
        REQUIRE(result == 124);
    }

    SECTION("parameter with const/reference") {
        mirrow::drefl::any a = Foo(123);
        auto& foo = mirrow::drefl::internal::invoke<clear_value>(&a, std::make_index_sequence<1>());
        REQUIRE(foo.value == 0);
        REQUIRE(&foo == &a.cast<Foo>());
    }

    SECTION("multi-parameter") {
        std::array<mirrow::drefl::any, 2> params = {
            mirrow::drefl::any(Foo(1)),
            mirrow::drefl::any(Foo(2)),
        };
        auto& foo = mirrow::drefl::internal::invoke<sum_value>(params.data(), std::make_index_sequence<2>());
        REQUIRE(&foo == &(params[0].cast<Foo>()));
        REQUIRE(foo.value == 3);
    }

    SECTION("type detect and cast") {
        {
            int integral = 123;
            mirrow::drefl::any a = integral;
            REQUIRE(a.type_info() == mirrow::drefl::reflected_type<int>().type_node());
            REQUIRE(a.try_cast_integral().has_value());
            REQUIRE(a.try_cast_integral().value() == 123);
            REQUIRE_FALSE(a.try_cast_uintegral());
            REQUIRE_FALSE(a.try_cast_floating_point());
        }

        {
            float f = 2.345f;
            mirrow::drefl::any a = f;
            REQUIRE(a.type_info() == mirrow::drefl::reflected_type<float>().type_node());
            REQUIRE_FALSE(a.try_cast_integral());
            REQUIRE(a.try_cast_floating_point());
            REQUIRE(a.try_cast_floating_point().value() == 2.345f);
            REQUIRE_FALSE(a.try_cast_uintegral());
        }

        {
            std::vector<int> value = {1, 2, 3, 4};
            mirrow::drefl::any a = value;
            REQUIRE(a.type_info() == mirrow::drefl::reflected_type<std::vector<int>>().type_node());
            REQUIRE_FALSE(a.try_cast_integral());
            REQUIRE_FALSE(a.try_cast_floating_point());
            REQUIRE_FALSE(a.try_cast_uintegral());

            int count = 0;
            a.travel_elements([&](mirrow::drefl::any& a){
                count += a.try_cast_integral().value();
            });
            REQUIRE(count == 10);
        }
    }
}
