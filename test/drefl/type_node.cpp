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
        auto info = mirrow::drefl::reflected_type<int>().type_node();
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
        auto info = mirrow::drefl::reflected_type<MyEnum>().type_node();
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
        auto info = mirrow::drefl::reflected_type<decltype(Foo)>().type_node();
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
        auto info = mirrow::drefl::reflected_type<decltype(&Clazz::Foo)>().type_node();
        REQUIRE(info->category == mirrow::drefl::type_category::MemberFunction);
        REQUIRE_FALSE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(info->name == "undefined");
    }

    SECTION("member object type") {
        auto info = mirrow::drefl::reflected_type<decltype(&Clazz::foo)>().type_node();
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
        auto info = mirrow::drefl::reflected_type<int[3]>().type_node();
        REQUIRE(info->category == mirrow::drefl::type_category::Compound);
        REQUIRE_FALSE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE_FALSE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE(info->is_array);
        REQUIRE(mirrow::drefl::reflected_type<int>().type_node() == info->raw_type);
    }

    SECTION("with qualifier") {
        auto info = mirrow::drefl::reflected_type<const int&>().type_node();
        REQUIRE(info->category == mirrow::drefl::type_category::Compound);
        REQUIRE(info->is_const);
        REQUIRE_FALSE(info->is_member_pointer);
        REQUIRE_FALSE(info->is_pointer);
        REQUIRE(info->is_reference);
        REQUIRE_FALSE(info->is_volatile);
        REQUIRE_FALSE(info->is_array);
        REQUIRE(mirrow::drefl::reflected_type<int>().type_node() == info->raw_type);

        auto type1 = mirrow::drefl::reflected_type<Clazz>();
        auto type2 = mirrow::drefl::reflected_type<const Clazz>().raw_type();
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz&>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz*>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz**>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz&>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<Clazz[3]>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz*>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz(&)[3]>().raw_type());
        REQUIRE(mirrow::drefl::reflected_type<Clazz>() == mirrow::drefl::reflected_type<const Clazz(*)[3]>().raw_type());
    }
}