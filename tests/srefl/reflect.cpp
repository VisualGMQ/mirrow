#include "mirrow/srefl/reflect.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

class Foo final {
public:
    void foo() {}
    void foo(int) {}
    void foo(float) {}
    void another() {}

    int value_1 = 1;
    int value_2 = 2;
};

// clang-format off
#include "mirrow/srefl/srefl_begin.hpp"
srefl_class(Foo,
    ctors()
    fields(
        field(static_cast<void(Foo::*)(void)>(&Foo::foo)),
        field(static_cast<void(Foo::*)(int)>(&Foo::foo)),
        field(static_cast<void(Foo::*)(float)>(&Foo::foo)),
        field(&Foo::another),
        field(&Foo::value_1),
        field(&Foo::value_2)
    )
)
#include "mirrow/srefl/srefl_end.hpp"
// clang-format on

using namespace mirrow::srefl;


TEST_CASE("type_info traits") {
    using info = ::mirrow::srefl::type_info<Foo>;
    static_assert(!has_ctors_v<info>);
    static_assert(has_fields_v<info>);
    static_assert(!has_bases_v<info>);
}

TEST_CASE("reflect") {
    auto refl = reflect<Foo>();
    REQUIRE(!refl.has_bases());
    REQUIRE(!refl.has_ctors());
    REQUIRE(refl.has_fields());

    std::vector<std::string_view> vars;
    refl.visit_member_variables([&vars](auto&& value) {
        vars.push_back(value.name());
    });
    REQUIRE(vars == std::vector<std::string_view>{"value_1", "value_2"});
}

enum class MyEnum {
    Value1 = 1,
    Value2 = 2,
    Value3 = 3,
};

// clang-format off
#include "mirrow/srefl/srefl_begin.hpp"
srefl_enum(MyEnum,
    enum_value(MyEnum::Value1, "Value1"),
    enum_value(MyEnum::Value2, "Value2"),
    enum_value(MyEnum::Value3, "Value3")
)
#include "mirrow/srefl/srefl_end.hpp"
    // clang-format on

TEST_CASE("reflect enum") {
    auto enum_type = reflect<MyEnum>();
    auto& values = enum_type.enum_values();

    REQUIRE(values[0].name == "Value1");
    REQUIRE(values[1].name == "Value2");
    REQUIRE(values[2].name == "Value3");
    REQUIRE(static_cast<int>(values[0].value) == 1);
    REQUIRE(static_cast<int>(values[1].value) == 2);
    REQUIRE(static_cast<int>(values[2].value) == 3);
}
