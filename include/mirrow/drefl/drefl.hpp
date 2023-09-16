#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/descriptor.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/info_node.hpp"

namespace mirrow {

namespace drefl {

template <typename... Args>
any invoke(function_descriptor& func, Args&&... args) {
    std::array<reference_any, sizeof...(Args)> params = {
        reference_any{std::forward<Args>(args)}...};
    return func.node()->invoke(params.data());
}

template <typename... Args>
any invoke(variable_descriptor& func, Args&&... args) {
    std::array<reference_any, sizeof...(Args)> params = {
        reference_any{std::forward<Args>(args)}...};
    return func.node()->invoke(params.data());
}

inline any invoke_by_any(const function_descriptor& func, basic_any* args) {
    return func.node()->invoke(args);
}

inline any invoke_by_any(const variable_descriptor& var, basic_any* args) {
    return var.node()->invoke(args);
}

inline any invoke_by_any(const ctor_descriptor& ctor, basic_any* args) {
    return ctor.node()->invoke(args);
}

inline reference_any invoke_by_any_return_ref(const function_descriptor& func, basic_any* args) {
    return func.node()->invoke_by_ref(args);
}

inline reference_any invoke_by_any_return_ref(const variable_descriptor& var, basic_any* args) {
    return var.node()->invoke_by_ref(args);
}

template <typename T>
mirrow::drefl::type_info reflected_type() {
    return mirrow::drefl::type_info{internal::info_node<T>::resolve()};
}

inline auto reflected_type(std::string_view name) {
    const auto nodes = internal::registry::nodes;
    auto it = std::find_if(
        nodes.begin(), nodes.end(),
        [&](internal::type_node* const n) { return n->name == name; });

    if (it != nodes.end()) {
        return mirrow::drefl::type_info{*it};
    } else {
        return mirrow::drefl::type_info{nullptr};
    }
}

}  // namespace drefl

}  // namespace mirrow
