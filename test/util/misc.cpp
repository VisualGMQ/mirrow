#include "mirrow/util/misc.hpp"
#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mirrow::util;

TEST_CASE("remove cvref") {
    REQUIRE(std::is_same_v<remove_cvref_t<int>, int>);
    REQUIRE(std::is_same_v<remove_cvref_t<int&>, int>);
    REQUIRE(std::is_same_v<remove_cvref_t<const int&>, int>);
    REQUIRE(std::is_same_v<remove_cvref_t<int&&>, int>);
    REQUIRE(std::is_same_v<remove_cvref_t<const int&&>, int>);
}

template <typename T>
struct show_tmpl;

TEST_CASE("remove all pointer") {
    REQUIRE(std::is_same_v<remove_all_pointers_t<int>, int>);
    REQUIRE(std::is_same_v<remove_all_pointers_t<int*>, int>);
    REQUIRE(std::is_same_v<remove_all_pointers_t<int**>, int>);
    REQUIRE(std::is_same_v<remove_all_pointers_t<int***>, int>);
    REQUIRE(std::is_same_v<remove_all_pointers_t<const int***>, const int>);
    REQUIRE(std::is_same_v<remove_all_pointers_t<const int*&>, const int*&>);
    REQUIRE(std::is_same_v<remove_all_pointers_t<const int*&&>, const int*&&>);
}

TEST_CASE("has qualifier") {
    REQUIRE_FALSE(has_qualifier_v<int>);
    REQUIRE_FALSE(has_qualifier_v<int*>);
    REQUIRE(has_qualifier_v<const int>);
    REQUIRE(has_qualifier_v<volatile int>);
}

struct Foo {
    void foo();
    int value;
};

TEST_CASE("is complex type") {
    REQUIRE_FALSE(is_complex_type_v<int>);
    REQUIRE(is_complex_type_v<const int>);
    REQUIRE(is_complex_type_v<int&>);
    REQUIRE(is_complex_type_v<const int&>);
    REQUIRE(is_complex_type_v<volatile int>);
    REQUIRE(is_complex_type_v<int*>);
    REQUIRE(is_complex_type_v<volatile int&>);
    REQUIRE(is_complex_type_v<int[3]>);
    REQUIRE_FALSE(is_complex_type_v<decltype(&Foo::foo)>);
    REQUIRE_FALSE(is_complex_type_v<decltype(&Foo::value)>);
}

TEST_CASE("strip type") {
    REQUIRE(std::is_same_v<strip_type_t<int>, int>);
    REQUIRE(std::is_same_v<strip_type_t<int&>, int>);
    REQUIRE(std::is_same_v<strip_type_t<int&&>, int>);
    REQUIRE(std::is_same_v<strip_type_t<const int>, int>);
    REQUIRE(std::is_same_v<strip_type_t<volatile int>, int>);
    REQUIRE(std::is_same_v<strip_type_t<int*>, int>);
    REQUIRE(std::is_same_v<strip_type_t<int**>, int>);
    REQUIRE(std::is_same_v<strip_type_t<const int**>, int>);
    REQUIRE(std::is_same_v<strip_type_t<const int&>, int>);
    REQUIRE(std::is_same_v<strip_type_t<const int&&>, int>);
    REQUIRE(std::is_same_v<strip_type_t<int[3]>, int>);
    REQUIRE(std::is_same_v<strip_type_t<decltype(&Foo::foo)>, decltype(&Foo::foo)>);
    REQUIRE(std::is_same_v<strip_type_t<decltype(&Foo::value)>, decltype(&Foo::value)>);
}

TEST_CASE("completely strip type") {
    REQUIRE(std::is_same_v<completely_strip_type_t<const int*&>, int>);
}

TEST_CASE("inner type") {
    REQUIRE(std::is_same_v<inner_type_t<std::optional<int>>, int>);
    REQUIRE(std::is_same_v<inner_type_t<std::vector<float>>, float>);
    REQUIRE(std::is_same_v<inner_type_t<std::unordered_map<float, int>>, std::pair<const float, int>>);
}
