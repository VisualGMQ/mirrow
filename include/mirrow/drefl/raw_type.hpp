#pragma once

#include "mirrow/util/misc.hpp"
#include "mirrow/util/variable_traits.hpp"

namespace mirrow::drefl {

template <typename T>
using property_raw_t = util::remove_cvref_t<util::remove_all_pointers_t<
    util::remove_cvref_t<typename util::variable_traits<T>::type>>>;

template <typename T>
using property_no_qualifier = util::remove_cvref_t<typename util::variable_traits<T>::type>;

template <typename T>
using raw_type_t =
    util::remove_cvref_t<util::remove_all_pointers_t<util::remove_cvref_t<T>>>;

}