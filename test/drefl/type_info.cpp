#include "mirrow/drefl/drefl.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

enum MyEnum {};

void Foo() {};

struct Clazz {
    void Foo() {}
    int foo;
};

template <typename T>
struct show_tmpl;

TEST_CASE("type info") {
    SECTION("without qualifier") {
        auto info = mirrow::drefl::reflected_type<int>();
        REQUIRE(info->category == mirrow::drefl::type_category::Fundamental);
        REQUIRE_FALSE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(info->name == "int");
    }

    SECTION("enum type") {
        auto info = mirrow::drefl::reflected_type<MyEnum>();
        REQUIRE(info->category == mirrow::drefl::type_category::Enum);
        REQUIRE_FALSE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(info->name == "undefined");
    }

    SECTION("normal function type") {
        auto info = mirrow::drefl::reflected_type<decltype(Foo)>();
        REQUIRE(info->category == mirrow::drefl::type_category::Function);
        REQUIRE_FALSE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(info->name == "undefined");
    }

    SECTION("member function type") {
        auto info = mirrow::drefl::reflected_type<decltype(&Clazz::Foo)>();
        REQUIRE(info->category == mirrow::drefl::type_category::MemberFunction);
        REQUIRE_FALSE(info->is_const);
        REQUIRE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(info->name == "undefined");
    }

    SECTION("member object type") {
        auto info = mirrow::drefl::reflected_type<decltype(&Clazz::foo)>();
        REQUIRE(info->category == mirrow::drefl::type_category::MemberObject);
        REQUIRE_FALSE(info->is_const);
        REQUIRE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(info->name == "undefined");
    }

    SECTION("with array") {
        auto info = mirrow::drefl::reflected_type<int[3]>();
        REQUIRE(info->category == mirrow::drefl::type_category::Fundamental);
        REQUIRE_FALSE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE(info->is_array);
        REQUIRE(mirrow::drefl::reflected_type<int>() == info->raw_type);
        REQUIRE(info->name == "int");
    }

    SECTION("with qualifier") {
        auto info = mirrow::drefl::reflected_type<const int&>();
        REQUIRE(info->category == mirrow::drefl::type_category::Fundamental);
        REQUIRE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(mirrow::drefl::reflected_type<int>() == info->raw_type);

        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz&>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz*>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz**>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz&>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz[3]>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz*>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz(&)[3]>()->raw_type);
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz(*)[3]>()->raw_type);
    }
}