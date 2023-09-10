#pragma once

#include <type_traits>
#include <utility>

namespace mirrow {

namespace util {

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

namespace detail {
    template <typename T>
    struct remove_all_pointers {
        using type = T;
    };

    template <typename T>
    struct remove_all_pointers<T*> {
        using type = typename remove_all_pointers<T>::type;
    };
}

/**
 * @brief remove all pointers from type
 */
template <typename T>
using remove_all_pointers_t = typename detail::remove_all_pointers<T>::type;

/**
 * @brief check whether a type has qualifier(const, volatile)
 */
template <typename T>
constexpr bool has_qualifier_v = std::is_volatile_v<T> || std::is_const_v<T>;

/**
 * @brief check whether a type has pointer, qualifier, reference or array
 */
template <typename T>
constexpr bool is_complex_type_v = has_qualifier_v<T> || std::is_pointer_v<T> ||
                                   std::is_array_v<T> || std::is_reference_v<T>;

/**
 * @brief remove qualifier/pointer/reference/array from type
 */
template <typename T>
using strip_type_t = remove_cvref_t<
    remove_all_pointers_t<std::decay_t<std::remove_all_extents_t<T>>>>;

namespace detail {

template <typename U>
[[maybe_unused]] static auto is_container_test(int)
    -> decltype(std::declval<U>().begin(), std::declval<U>().end(),
                std::true_type{});

template <typename U>
[[maybe_unused]] static std::false_type is_container_test(...);

/**
 * @brief check whether a type is a container(has begin() and end() member
 * function)
 */
template <typename T>
struct is_container : decltype(detail::is_container_test<T>(0)) {};


template <typename T, typename = void>
struct completely_strip_type {
    using type = T;
};

template <typename T>
struct completely_strip_type<T, std::enable_if_t<is_complex_type_v<T>>> {
    using type = typename completely_strip_type<strip_type_t<T>>::type;
};

}  // namespace detail

/**
 * Checks whether objects of the type T support member .begin() and .end()
 * operations.
 */
template <typename T>
constexpr bool is_container_v = detail::is_container<T>::value;

/**
 * @brief remove all qualifier/pointer/reference/array from type
 */
template <typename T>
using completely_strip_type_t = typename detail::completely_strip_type<T>::type;

}  // namespace util

}  // namespace mirrow