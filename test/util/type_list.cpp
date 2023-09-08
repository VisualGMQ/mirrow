#include "mirrow/util/type_list.hpp"

#include <tuple>
#include <utility>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mirrow::util;

using list = type_list<int, char, float, double>;
using tuple_list = std::tuple<int, char, float, double>;

TEST_CASE("get list element") {
    SECTION("in type_list") {
        static_assert(std::is_same_v<list_element_t<list, 0>, int>);
        static_assert(std::is_same_v<list_element_t<list, 1>, char>);
        static_assert(std::is_same_v<list_element_t<list, 2>, float>);
        static_assert(std::is_same_v<list_element_t<list, 3>, double>);
        static_assert(std::is_same_v<list_head_t<list>, int>);
        static_assert(list_size_v<list> == 4);
    }

    SECTION("in std::tuple") {
        static_assert(std::is_same_v<list_element_t<tuple_list, 0>, int>);
        static_assert(std::is_same_v<list_element_t<tuple_list, 1>, char>);
        static_assert(std::is_same_v<list_element_t<tuple_list, 2>, float>);
        static_assert(std::is_same_v<list_element_t<tuple_list, 3>, double>);
        static_assert(std::is_same_v<list_head_t<tuple_list>, int>);
        static_assert(list_size_v<tuple_list> == 4);
    }
}

template <typename T>
struct IsInt {
    static constexpr bool value = false;
};

template <>
struct IsInt<int> {
    static constexpr bool value = true;
};

template <typename T>
void FilterEven(T value) {
    return value % 2 == 0;
}

TEST_CASE("utility functions") {
    static_assert(disjunction_v<list, IsInt>);
    static_assert(!conjunction_v<list, IsInt>);
    static_assert(std::is_same_v<typelist_to_tuple_t<list>, tuple_list>);
    static_assert(std::is_same_v<tuple_to_typelist<tuple_list>, list>);

    static_assert(std::is_same_v<list_filter_t<list, std::is_integral>, type_list<int, char>>);
    static_assert(std::is_same_v<list_filter_t<type_list<>, std::is_integral>, type_list<>>);
}
