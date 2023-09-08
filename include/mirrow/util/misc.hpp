#pragma once

#include <type_traits>
#include <utility>

namespace mirrow {

namespace util {

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

namespace detail {

template <typename U>
[[maybe_unused]] static auto is_container_test(int)
    -> decltype(std::declval<U>().begin(), std::declval<U>().end(),
                std::true_type{});

template <typename U>
[[maybe_unused]] static std::false_type is_container_test(...);

template <typename T>
struct is_container : decltype(detail::is_container_test<T>(0)) {};

}  // namespace detail

/**
 * Checks whether objects of the type T support member .begin() and .end()
 * operations.
 */
template <typename T>
constexpr bool is_container_v = detail::is_container<T>::value;

}  // namespace util

}  // namespace mirrow