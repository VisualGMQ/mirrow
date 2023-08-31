#pragma once

#include "mirrow/srefl/srefl.hpp"

namespace mirrow {

namespace srefl {

template <typename T>
class reflect final {
public:
    using type = type_info<T>;
};

}

}