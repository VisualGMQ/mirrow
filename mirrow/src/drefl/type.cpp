#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/array.hpp"
#include "mirrow/drefl/bool.hpp"
#include "mirrow/drefl/class.hpp"
#include "mirrow/drefl/enum.hpp"
#include "mirrow/drefl/numeric.hpp"
#include "mirrow/drefl/pointer.hpp"
#include "mirrow/drefl/string.hpp"
#include "mirrow/drefl/optional.hpp"
#include "mirrow/drefl/value_kind.hpp"

namespace mirrow::drefl {

const clazz* type::as_class() const noexcept {
    if (kind() == value_kind::Class) {
        return static_cast<const clazz*>(this);
    }
    return nullptr;
}

const numeric* type::as_numeric() const noexcept {
    if (kind() == value_kind::Numeric) {
        return static_cast<const numeric*>(this);
    }
    return nullptr;
}

const enum_info* type::as_enum() const noexcept {
    if (kind() == value_kind::Enum) {
        return static_cast<const enum_info*>(this);
    }
    return nullptr;
}

const boolean* type::as_boolean() const noexcept {
    if (kind() == value_kind::Boolean) {
        return static_cast<const boolean*>(this);
    }
    return nullptr;
}

const string* type::as_string() const noexcept {
    if (kind() == value_kind::String) {
        return static_cast<const string*>(this);
    }
    return nullptr;
}

const pointer* type::as_pointer() const noexcept {
    if (kind() == value_kind::Pointer) {
        return static_cast<const pointer*>(this);
    }

    return nullptr;
}

const array* type::as_array() const noexcept {
    if (kind() == value_kind::Array) {
        return static_cast<const array*>(this);
    }

    return nullptr;
}

const optional* type::as_optional() const noexcept {
    if (kind() == value_kind::Optional) {
        return static_cast<const optional*>(this);
    }
    return nullptr;
}

bool type::is_class() const noexcept {
    return kind_ == value_kind::Class;
}

bool type::is_numeric() const noexcept {
    return kind_ == value_kind::Numeric;
}

bool type::is_enum() const noexcept {
    return kind_ == value_kind::Enum;
}

bool type::is_boolean() const noexcept {
    return kind_ == value_kind::Boolean;
}

bool type::is_string() const noexcept {
    return kind_ == value_kind::String;
}

bool type::is_pointer() const noexcept {
    return kind_ == value_kind::Pointer;
}

bool type::is_array() const noexcept {
    return kind_ == value_kind::Array;
}

bool type::is_optional() const noexcept {
    return kind_ == value_kind::Optional;
}

}  // namespace mirrow::drefl