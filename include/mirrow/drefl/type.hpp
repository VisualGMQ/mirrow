#pragma once

#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/drefl/any.hpp"

namespace mirrow::drefl {

class clazz;
class numeric;
class enum_info;
class boolean;
class string;
class pointer;
class array;
class optional;

struct type {
    explicit type(value_kind kind, const std::string& name)
        : kind_(kind), name_(name) {}
    explicit type(value_kind kind): kind_(kind) {}
    virtual ~type() = default;

    auto kind() const noexcept { return kind_; }

    const clazz* as_class() const noexcept;
    const numeric* as_numeric() const noexcept;
    const enum_info* as_enum() const noexcept;
    const boolean* as_boolean() const noexcept;
    const string* as_string() const noexcept;
    const pointer* as_pointer() const noexcept;
    const array* as_array() const noexcept;
    const optional* as_optional() const noexcept;

    bool is_class() const noexcept;
    bool is_numeric() const noexcept;
    bool is_enum() const noexcept;
    bool is_boolean() const noexcept;
    bool is_string() const noexcept;
    bool is_pointer() const noexcept;
    bool is_array() const noexcept;
    bool is_optional() const noexcept;

    auto& name() const noexcept { return name_; }

protected:
    std::string name_;

private:
    value_kind kind_;
};

#define SET_VALUE_CHECK(a, type)                     \
    ((a.access_type() == any::access_type::Ref ||   \
      a.access_type() == any::access_type::Copy) && \
     a.type_info()->kind() == type)

#define COPY_VALUE_CHECK(a, type)                       \
    (a.access_type() != any::access_type::Null && \
     a.type_info()->kind() == type)

}  // namespace mirrow::drefl