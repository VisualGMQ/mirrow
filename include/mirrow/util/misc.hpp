#pragma once

#include <type_traits>

namespace mirrow {

namespace util {

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

}

}