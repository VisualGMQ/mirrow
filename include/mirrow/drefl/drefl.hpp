#pragma once

#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/info_node.hpp"
#include "mirrow/drefl/descriptor.hpp"

namespace mirrow {

namespace drefl {

inline any invoke_by_any(function_descriptor& func, any* args) {
    return func.node_->invoke(args);
}

inline any invoke_by_any(variable_descriptor& var, any* args)  {
    return var.node_->invoke(args);
}

template <typename T>
auto reflected_type() {
    return mirrow::drefl::type_info{internal::info_node<T>::resolve()};
}

auto reflected_type(std::string_view name) {
    const auto nodes = internal::registry::nodes;
    auto it = std::find_if(nodes.begin(), nodes.end(), [&](internal::type_node*const n) {
        return n->name == name;
    });

    if (it != nodes.end()) {
        return mirrow::drefl::type_info{*it};
    } else {
        return mirrow::drefl::type_info{nullptr};
    }
}

}  // namespace drefl

}  // namespace mirrow