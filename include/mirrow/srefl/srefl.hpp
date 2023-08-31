#pragma once

#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/type_list.hpp"
#include "mirrow/util/variable_traits.hpp"

#include <string_view>
#include <utility>

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
 * @brief strip class/function/variable name from namespace/class prefix to pure name
 */
inline constexpr std::string_view strip_name(const std::string_view name) {
    auto idx = name.find_last_of(':');
    if (idx == std::string_view::npos) {
        idx = name.find_last_of('&');
        if (idx == std::string_view::npos) {
            return name;
        } else {
            return name.substr(idx + 1, name.length());
        }
    } else {
        return name.substr(idx + 1, name.length());
    }
}

/**
 * @brief extract class field(member variable, member function) info
 *
 * @tparam T
 * @tparam Attrs
 */
template <typename T, typename... Attrs>
struct field_traits : internal::basic_field_traits<T, util::is_function_v<T>> {
    explicit constexpr field_traits(T&& pointer, std::string_view name,
                                    Attrs&&... attrs)
        : pointer(std::forward<T>(pointer)),
          name(strip_name(name)),
          attrs(std::forward<Attrs>(attrs)...) {}

    using traits = field_traits<T>;

    T pointer;
    std::string_view name;
    std::tuple<Attrs...> attrs;
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
template <typename T, typename... attrs>
struct type_info;

}  // namespace srefl

}  // namespace mirrow