#pragma once

#include "mirrow/util/type_list.hpp"

namespace mirrow {

namespace util {

namespace detail {

template <typename F>
struct function_type;

template <typename Ret, typename... Args>
struct function_type<Ret (*)(Args...)> {
    using type = Ret(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_type<Ret (Class::*)(Args...)> {
    using type = Ret (Class::*)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_type<Ret (Class::*)(Args...) const> {
    using type = Ret (Class::*)(Args...) const;
};

}  // namespace detail

/**
 * @brief get function type from function pointer type
 *
 * @tparam F
 */
template <typename F>
using function_type_t = typename detail::function_type<F>::type;

namespace detail {

template <typename Ret, typename... Args>
auto function_pointer_to_type(int, Ret (*)(Args...)) -> Ret (*)(Args...);

template <typename Ret, typename Class, typename... Args>
auto function_pointer_to_type(char, Ret (Class::*)(Args...))
    -> Ret (Class::*)(Args...);

template <typename Ret, typename Class, typename... Args>
auto function_pointer_to_type(char, Ret (Class::*)(Args...) const)
    -> Ret (Class::*)(Args...) const;

}  // namespace detail

/**
 * @brief get a function pointer type from it's function pointer
 *
 * @tparam F
 */
template <auto F>
using function_pointer_type_t =
    decltype(detail::function_pointer_to_type(0, F));

/**
 * @brief get a function type from it's function pointer
 *
 * @tparam F
 */
template <auto F>
using function_type_from_pointer_t =
    function_type_t<decltype(detail::function_pointer_to_type(0, F))>;

namespace detail {

template <typename Func>
struct basic_function_traits;

template <typename Ret, typename... Args>
struct basic_function_traits<Ret(Args...)> {
    using args = util::type_list<Args...>;
    using return_type = Ret;
};

}  // namespace detail

/**
 * @brief extract function info from function type
 *
 * @tparam Func
 */
template <typename Func>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)>
    : detail::basic_function_traits<Ret(Args...)> {
    using type = Ret(Args...);
    using args_with_class = type_list<Args...>;
    using pointer = Ret(*)(Args...);
    static constexpr bool is_member = false;
    static constexpr bool is_const = false;
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...)>
    : detail::basic_function_traits<Ret(Args...)> {
    using type = Ret (Class::*)(Args...);
    using args_with_class = type_list<Class*, Args...>;
    using pointer = Ret (Class::*)(Args...);
    static constexpr bool is_member = true;
    static constexpr bool is_const = false;
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...) const>
    : detail::basic_function_traits<Ret(Args...)> {
    using type = Ret (Class::*)(Args...) const;
    using args_with_class = type_list<Class*, Args...>;
    using pointer = Ret (Class::*)(Args...) const;
    static constexpr bool is_member = true;
    static constexpr bool is_const = true;
};

namespace detail {

template <auto F>
struct function_pointer_traits
    : function_traits<function_type_from_pointer_t<F>> {};

}  // namespace detail

/**
 * @brief extract function info from function pointer
 *
 * @tparam F
 */
template <auto F>
using function_pointer_traits = detail::function_pointer_traits<F>;

/**
 * @brief check a type is a function or function pointer
 */
template <typename T>
constexpr bool is_function_v = std::is_function_v<T> || std::is_member_function_pointer_v<T>;

}  // namespace util

}  // namespace mirrow