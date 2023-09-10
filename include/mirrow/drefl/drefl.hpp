#pragma once

#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/info_node.hpp"
// #include "mirrow/drefl/refl_info.hpp"

namespace mirrow {

namespace drefl {

/*

template <typename T>
refl_info meta_info() {
    return refl_info(internal::info_node<T>::type);
}

refl_info meta_info(std::string_view name) {
    for (auto node : internal::registry::nodes) {
        if (node->name == name) {
            return refl_info(node);
        }
    }

    return refl_info(nullptr);
}

*/

template <typename T>
internal::type_node* reflected_type() {
    return internal::info_node<T>::resolve();
}

}  // namespace drefl

}  // namespace mirrow