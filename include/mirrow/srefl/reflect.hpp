#pragma once

#include "mirrow/srefl/srefl.hpp"
#include "mirrow/util/type_list.hpp"
#include "mirrow/util/misc.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

namespace mirrow {

namespace srefl {

// some type_info traits
namespace detail {

template <typename TypeInfo, typename = std::void_t<>>
struct has_bases final : std::false_type {};

template <typename TypeInfo>
struct has_bases<TypeInfo, std::void_t<typename TypeInfo::bases>> {
    static constexpr bool value =
        !util::is_list_empty_v<typename TypeInfo::bases>;
};

template <typename TypeInfo, typename = std::void_t<>>
struct has_fields final : std::false_type {};

template <typename TypeInfo>
struct has_fields<TypeInfo, std::void_t<decltype(TypeInfo::fields)>> {
    static constexpr bool value = !util::is_list_empty_v<
        std::remove_cv_t<std::remove_const_t<decltype(TypeInfo::fields)>>>;
};

template <typename TypeInfo, typename = std::void_t<>>
struct has_ctors final : std::false_type {};

template <typename TypeInfo>
struct has_ctors<TypeInfo, std::void_t<typename TypeInfo::ctors>> {
    static constexpr bool value =
        !util::is_list_empty_v<typename TypeInfo::ctors>;
};

}  // namespace detail

namespace internal {

template <size_t... Idx, typename TupleType>
constexpr auto pick_tuple_elements(TupleType&& tuple,
                                   std::index_sequence<Idx...>) {
    return std::make_tuple(std::get<Idx>(tuple)...);
}

template <size_t... Idx>
constexpr auto inc_seq_elem(std::index_sequence<Idx...> seq) {
    return std::index_sequence<(Idx + 1)...>{};
}

}  // namespace internal

/**
 * @brief get tail of tuple(the elems without first elem)
 */
template <typename TupleType>
constexpr auto tuple_tail(TupleType&& tuple) {
    using tuple_type = util::remove_cvref_t<TupleType>;

    if constexpr (util::list_size_v<tuple_type> >= 1) {
        return internal::pick_tuple_elements(
            std::forward<TupleType>(tuple),
            internal::inc_seq_elem(
                std::make_index_sequence<util::list_size_v<tuple_type> - 1>{}));
    } else {
        return std::tuple<>{};
    }
}

/**
 * @brief check whether a type_info has bases classes
 */
template <typename TypeInfo>
constexpr bool has_bases_v = detail::has_bases<TypeInfo>::value;

/**
 * @brief check whether a type_info has ctors
 */
template <typename TypeInfo>
constexpr bool has_ctors_v = detail::has_ctors<TypeInfo>::value;

/**
 * @brief check whether a type_info has field
 */
template <typename TypeInfo>
constexpr bool has_fields_v = detail::has_fields<TypeInfo>::value;

template <typename T, typename Field>
struct field_descriptor {
    using clazz = T;
    using field = Field;
};

template <typename T>
class reflect_info final {
public:
    using type = type_info<T>;

    /**
     * @brief construct a instance
     */
    template <typename... Args>
    T construct(Args&&... args) {
        return T{std::forward<Args>(args)...};
    }

    /**
     * @brief check whether the type is class
     */
    constexpr bool is_class() const noexcept {
        return std::is_class_v<T>;
    }

    /**
     * @brief check whether the type is enum
     */
    constexpr bool is_enum() const noexcept {
        return std::is_enum_v<T>;
    }

    /**
     * @brief check whether class has base classes
     */
    constexpr bool has_bases() const noexcept { return has_bases_v<type>; }

    /**
     * @brief check whether class has constructors
     */
    constexpr bool has_ctors() const noexcept { return has_ctors_v<type>; }

    /**
     * @brief check whether class has fields
     */
    constexpr bool has_fields() const noexcept { return has_fields_v<type>; }

    constexpr decltype(auto) enum_values() const noexcept {
        if constexpr (std::is_enum_v<T>) {
            return type::enums;
        } else {
            return std::array<enum_value<int>, 0>{};
        }
    }

    /**
     * @brief runtime tool: visit all fields
     */
    template <typename Function>
    void visit_fields(Function&& func) {
        if constexpr (has_fields_v<type>) {
            std::apply(
                [&func](auto&&... args) {
                    (func(std::forward<decltype(args)>(args)), ...);
                },
                type::fields);
        }
    }

    /**
     * @brief runtime tool: visit all member variables
     */
    template <typename Function>
    void visit_member_variables(Function&& func) {
        if constexpr (has_fields_v<type>) {
            do_visit_member_variables<0>(std::forward<Function>(func));
        }
    }

    constexpr std::string_view name() const noexcept {
        return type::name();
    }

    /**
     * @brief runtime tool: visit all member functions
     */
    template <typename Function>
    void visit_member_functions(Function&& func) {
        if constexpr (has_fields_v<type>) {
            do_visit_member_functions<0>(std::forward<Function>(func));
        }
    }

private:
    template <size_t Idx, typename Function>
    void do_visit_member_variables(Function&& func) {
        auto fields = type::fields;
        if constexpr (Idx < util::list_size_v<std::remove_cv_t<
                                std::remove_reference_t<decltype(fields)>>>) {
            auto field = std::get<Idx>(fields);
            if constexpr (field.is_variable() && field.is_member()) {
                func(std::get<Idx>(fields));
            }
            do_visit_member_variables<Idx + 1>(std::forward<Function>(func));
        }
    }

    template <size_t Idx, typename Function>
    void do_visit_member_functions(Function&& func) {
        constexpr auto fields = type::fields;
        if constexpr (Idx < util::list_size_v<std::remove_cv_t<
                                std::remove_reference_t<decltype(fields)>>>) {
            constexpr auto field = std::get<Idx>(fields);
            if constexpr (field.is_function() && field.is_member()) {
                func(std::get<Idx>(fields));
            }
            do_visit_member_functions<Idx + 1>(std::forward<Function>(func));
        }
    }
};

/**
 * @brief get reflected class info
 */
template <typename T>
constexpr auto reflect() {
    return reflect_info<util::remove_cvref_t<T>>{};
}

}  // namespace srefl

}  // namespace mirrow