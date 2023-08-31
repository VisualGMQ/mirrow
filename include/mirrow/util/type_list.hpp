#pragma once

#include <tuple>
#include <cstdint>

namespace mirrow {

namespace util {

/**
 * @brief a type container to store types
 *
 * @tparam Ts types
 */
template <typename... Ts>
struct type_list {
    using self_type = type_list<Ts...>;
    static constexpr size_t size = sizeof...(Ts);
};

namespace detail {

template <typename, size_t>
struct list_element;

template <template <typename...> typename ListType, typename T, typename... Ts,
          size_t N>
struct list_element<ListType<T, Ts...>, N>
    : list_element<ListType<Ts...>, N - 1> {};

template <template <typename...> typename ListType, typename T, typename... Ts>
struct list_element<ListType<T, Ts...>, 0> {
    using type = T;
};

template <typename>
struct list_size;

template <template <typename...> typename ListType, typename... Ts>
struct list_size<ListType<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

template <typename>
struct list_head;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct list_head<ListType<T, Ts...>> {
    using type = T;
};

template <typename>
struct list_tail;

template <template <typename...> typename ListType, typename T, typename... Ts>
struct list_tail<ListType<T, Ts...>> {
    using type = ListType<Ts...>;
};

template <typename List, typename T>
struct list_add_to_first;

template <template <typename...> typename ListType, typename... Ts, typename T>
struct list_add_to_first<ListType<Ts...>, T> {
    using type = ListType<T, Ts...>;
};

}  // namespace detail

/**
 * @brief get the N-th element in type list(from type_list or std::tuple or
 * others)
 *
 * @tparam N the element index
 * @tparam List
 */
template <typename List, size_t N>
using list_element_t = typename detail::list_element<List, N>::type;

/**
 * @brief get length of type list
 *
 * @tparam List
 */
template <typename List>
constexpr size_t list_size_v = detail::list_size<List>::value;

/**
 * @brief check whether a list is empty
 *
 * @tparam List
 */
template <typename List>
constexpr bool is_list_empty = list_size_v<List> == 0;

/**
 * @brief get first element from type list
 *
 * @tparam List
 */
template <typename List>
using list_head_t = typename detail::list_head<List>::type;

/**
 * @brief get tail of type list
 *
 * @tparam List
 */
template <typename List>
using list_tail_t = typename detail::list_tail<List>::type;

template <typename List, typename T>
using list_add_to_first_t = typename detail::list_add_to_first<List, T>::type;

namespace detail {

template <typename List, size_t N, template <typename> typename F>
struct apply_to_element {
    using type = F<util::list_element_t<List, N>>;
};

}  // namespace detail

/**
 * @brief apply a template function struct to a list element
 *
 * @tparam List
 * @tparam N
 * @tparam F  like `template<typename> struct { constexpr auto value = XXX };`,
 * recieve a type and give a value
 */
template <typename List, size_t N, template <typename> typename F>
using apply_to_element_t = typename detail::apply_to_element<List, N, F>::type;

namespace detail {

template <typename List, template <typename> typename F>
struct disjunction {
    static constexpr bool value =
        F<util::list_head_t<List>>::value ||
        disjunction<util::list_tail_t<List>, F>::value;
};

template <template <typename...> typename ListType,
          template <typename> typename F>
struct disjunction<ListType<>, F> {
    static constexpr bool value = false;
};

template <typename List, template <typename> typename F>
struct conjunction {
    static constexpr bool value =
        F<util::list_head_t<List>>::value &&
        disjunction<util::list_tail_t<List>, F>::value;
};

template <template <typename...> typename ListType,
          template <typename> typename F>
struct conjunction<ListType<>, F> {
    static constexpr bool value = true;
};

template <typename List1, typename List2>
struct concat;

template <template <typename...> typename ListType, typename... Ts1,
          typename... Ts2>
struct concat<ListType<Ts1...>, ListType<Ts2...>> {
    using type = ListType<Ts1..., Ts2...>;
};

template <typename TypeList>
struct typelist_to_tuple;

template <typename... Ts>
struct typelist_to_tuple<type_list<Ts...>> {
    using type = std::tuple<Ts...>;
};

template <typename Tuple>
struct tuple_to_typelist;

template <typename... Ts>
struct tuple_to_typelist<std::tuple<Ts...>> {
    using type = type_list<Ts...>;
};

template <typename List, template <typename> typename F>
struct list_filter;

template <template <typename...> typename ListType, typename T, typename... Ts,
          template <typename> typename F>
struct list_filter<ListType<T, Ts...>, F> {
    using type = std::conditional_t<
        F<T>::value,
        list_add_to_first_t<typename list_filter<ListType<Ts...>, F>::type, T>,
        typename list_filter<ListType<Ts...>, F>::type>;
};

template <template <typename...> typename ListType,
          template <typename> typename F>
struct list_filter<ListType<>, F> {
    using type = ListType<>;
};

}  // namespace detail

/**
 * @brief work as std::disjunction: use function struct F to check any of
 * element is true
 *
 * @tparam List
 * @tparam F  receive a type and give a constexpr static bool value;
 */
template <typename List, template <typename> typename F>
constexpr bool disjunction_v = detail::disjunction<List, F>::value;

/**
 * @brief work as std::conjunction: use function struct F to check all of
 * element is true
 *
 * @tparam List
 * @tparam F  receive a type and give a constexpr static bool value;
 */
template <typename List, template <typename> typename F>
constexpr bool conjunction_v = detail::conjunction<List, F>::value;

/**
 * @brief concat two type list
 *
 * @tparam List1
 * @tparam List2
 */
template <typename List1, typename List2>
using concat_t = typename detail::concat<List1, List2>::type;

template <typename TypeList>
using typelist_to_tuple_t = typename detail::typelist_to_tuple<TypeList>::type;

template <typename Tuple>
using tuple_to_typelist = typename detail::tuple_to_typelist<Tuple>::type;

template <typename List, template <typename> typename F>
using list_filter_t = typename detail::list_filter<List, F>::type;

}  // namespace utility

}  // namespace mirrow
