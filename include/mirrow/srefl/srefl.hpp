#pragma once

#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/type_list.hpp"
#include "mirrow/util/variable_traits.hpp"

#include <string_view>
#include <utility>
#include <functional>

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
struct basic_field_traits<T, true> : util::function_traits<T> {
    constexpr bool is_const_member() const noexcept {
        return util::function_traits<T>::is_const;
    }

    constexpr bool is_member() const noexcept {
        return util::function_traits<T>::is_member;
    }

    constexpr bool is_function() const noexcept {
        return true;
    }

    constexpr bool is_variable() const noexcept {
        return false;
    }
};

template <typename T>
struct basic_field_traits<T, false> : util::variable_traits<T> {
    constexpr bool is_const_member() const noexcept {
        return false;
    }

    constexpr bool is_member() const noexcept {
        return util::variable_traits<T>::is_member;
    }

    constexpr bool is_function() const noexcept {
        return false;
    }

    constexpr bool is_variable() const noexcept {
        return true;
    }
};

}  // namespace internal

/**
 * @brief strip class/function/variable name from namespace/class prefix to pure name
 */
inline constexpr std::string_view strip_name(std::string_view name) {
    std::string_view result = name;

    if (auto idx = name.find_last_of('&'); idx != std::string_view::npos) {
        name = name.substr(idx + 1, name.length());
    }
    if (auto idx = name.find_last_of(':'); idx != std::string_view::npos) {
        name = name.substr(idx + 1, name.length());
    }
    if (auto idx = name.find_first_of(')'); idx != std::string_view::npos) {
        name = name.substr(0, idx);
    }

    return name;
}

/**
 * @brief extract class field(member variable, member function) info
 *
 * @tparam T type
 * @tparam Attrs attributes
 */
template <typename T, typename... Attrs>
struct field_traits : internal::basic_field_traits<T, util::is_function_v<T>> {
    constexpr field_traits(T&& pointer, std::string_view name,
                                    Attrs&&... attrs)
        : pointer_(std::forward<T>(pointer)),
          name_(strip_name(name)),
          attrs_(std::forward<Attrs>(attrs)...) {}

    /**
     * @brief check whether field is a const member(class const function)
     */
    constexpr bool is_const_member() const noexcept {
        return base::is_const_member();
    }

    /**
     * @brief check whether field is class member or static/global
     */
    constexpr bool is_member() const noexcept {
        return base::is_member();
    }

    /**
     * @brief get field name
     */
    constexpr std::string_view name() const noexcept {
        return name_;
    }

    /**
     * @brief get pointer
     */
    constexpr auto pointer() const noexcept {
        return pointer_;
    }

    /**
     * @brief get attributes
     */
    constexpr auto& attrs() const noexcept {
        return attrs_;
    }

    template <typename... Args>
    auto invoke(Args&&... args) {
        if constexpr (!util::is_function_v<T>) {
            if constexpr (util::variable_traits<T>::is_member) {
                return std::invoke(this->pointer_, std::forward<Args>(args)...);
            } else {
                return *(this->pointer_);
            }
        } else {
            return std::invoke(this->pointer_, std::forward<Args>(args)...);
        }
    }

private:
    using base = internal::basic_field_traits<T, util::is_function_v<T>>;

    T pointer_;
    std::string_view name_;
    std::tuple<Attrs...> attrs_;
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