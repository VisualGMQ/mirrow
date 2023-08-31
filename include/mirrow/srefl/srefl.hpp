#pragma once

#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/type_list.hpp"
#include "mirrow/util/variable_traits.hpp"

#include <utility>
#include <string_view>

namespace mirrow {

namespace srefl {

/**
 * @brief attributes that attach to field/function
 */
template <typename... Attrs>
using attr_list = util::type_list<Attrs...>;

namespace internal {

template <typename T, bool>
struct basic_field_traits;

template <typename T>
struct basic_field_traits<T, true> : util::function_traits<T> {};

template <typename T>
struct basic_field_traits<T, false> : util::variable_traits<T> {};

}  // namespace internal

/**
 * @brief extract class field(member variable, member function) info
 *
 * @tparam T
 * @tparam Attrs
 */
template <auto T, typename AttrList = attr_list<>, typename Next = void,
          typename Type = decltype(T)>
struct field_traits : internal::basic_field_traits<Type, util::is_function_v<Type>> {
    using next = Next;
    inline static constexpr auto pointer = T;
};

/**
 * @brief store class constructor
 */
template <typename... Args>
struct ctor {
    using args = util::type_list<Args...>;
};

/**
 * @brief store base classes
 */
template <typename... Bases>
struct base {
    using bases = util::type_list<Bases...>;
};

template <typename T>
struct base_type_info {
    using type = T;
    static constexpr bool is_final = std::is_final_v<T>;
};

/**
 * @brief store class type info
 *
 * @tparam T type
 * @tparam AttrList attributes
 */
template <typename T>
struct type_info;

}  // namespace srefl

}  // namespace mirrow