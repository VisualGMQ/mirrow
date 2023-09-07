#pragma once

#include "mirrow/drefl/info_node.hpp"
#include "mirrow/drefl/refl_info.hpp"
#include "mirrow/drefl/factory.hpp"

namespace mirrow {

namespace drefl {

template <typename T>
refl_info resolve() {
    return refl_info(internal::info_node<T>::type);
}

}

}