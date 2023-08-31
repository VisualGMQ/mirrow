#pragma once

#include <type_traits>

namespace mirrow {

namespace util {

namespace detail {

template <typename T>
struct variable_type {
    using type = T;
};

template <typename Class, typename T>
struct variable_type<T Class::*> {
    using type = T;
};

}  // namespace detail

/**
 * @brief get variable type
 * simple variable: same as T
 * class variable: variable type
 * 
 * @tparam T a variable type
 */
template <typename T>
using variable_type_t = typename detail::variable_type<T>::type;

namespace detail {

template <typename T>
auto variable_pointer_to_type(long, T*) -> T;

template <typename Class, typename T>
auto variable_pointer_to_type(char, T Class::*) -> T;

}  // namespace detail

/**
 * @brief get a variable type from it's variable pointer
 */
template <auto V>
using variable_pointer_to_type_t =
    decltype(detail::variable_pointer_to_type(0, V));

namespace internal {

template <typename T>
struct basic_variable_traits {
    using type = variable_type_t<T>;
    static constexpr bool is_member = std::is_member_pointer_v<T>;
};

}  // namespace internal

/**
 * @brief extract variable info from variable type
 *
 * @tparam Func
 */
template <typename T>
struct variable_traits;

template <typename T>
struct variable_traits<T*>
    : internal::basic_variable_traits<T> {
    using pointer = T*;
};

template <typename Class, typename T>
struct variable_traits<T Class::*> : internal::basic_variable_traits<T Class::*> {
    using pointer = T Class::*;
};

namespace detail {

template <auto V>
struct variable_pointer_traits
    : variable_traits<decltype(V)> {};

}  // namespace detail

/**
 * @brief extract variable info from variable pointer
 *
 * @tparam F
 */
template <auto V>
using variable_pointer_traits = detail::variable_pointer_traits<V>;

}  // namespace util

}  // namespace mirrow