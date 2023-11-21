#pragma once

#include <string_view>
#include <string>
#include <type_traits>
#include "mirrow/util/misc.hpp"

namespace mirrow::drefl {

enum class value_kind {
    None,
    Boolean,
    Numeric,
    String,
    Enum,
    Class,
    Property,
    Pointer,
    Array,
    Optional,
};

template <typename T>
value_kind get_kind_from_type() {
    if constexpr (util::is_std_array_v<T> || util::is_vector_v<T> ||
                  std::is_array_v<T>) {
        return value_kind::Array;
    }
    if constexpr (util::is_optional_v<T>) {
        return value_kind::Optional;
    }
    if constexpr (std::is_pointer_v<T>) {
        return value_kind::Pointer;
    }
    if constexpr (std::is_fundamental_v<T>) {
        return value_kind::Numeric;
    }
    if constexpr (std::is_same_v<T, std::string> ||
                  std::is_same_v<T, std::string_view>) {
        return value_kind::String;
    }
    if constexpr (std::is_enum_v<T>) {
        return value_kind::Enum;
    }
    if constexpr (std::is_class_v<T>) {
        return value_kind::Class;
    }

    return value_kind::None;
}

}  // namespace mirrow::drefl