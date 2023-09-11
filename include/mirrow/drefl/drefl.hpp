#pragma once

#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/info_node.hpp"
#include "mirrow/drefl/descriptor.hpp"

namespace mirrow {

namespace drefl {

template <typename T>
auto reflected_type() {
    return mirrow::drefl::type_info{internal::info_node<T>::resolve()};
}

}  // namespace drefl

}  // namespace mirrow