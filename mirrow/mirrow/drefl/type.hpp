#pragma once

#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/config.hpp"
#include <algorithm>
#include <functional>

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
    using default_construct_fn = std::function<any(void)>;

    type(value_kind kind, const std::string& name, default_construct_fn fn)
        : kind_(kind), name_(name), default_construct_(fn) {}
    type(value_kind kind, default_construct_fn fn): kind_(kind), default_construct_(fn) {}
    virtual ~type() = default;

    auto& attributes() const { return attrs_; }
    void set_attr(const std::vector<attribute_t>& attrs) { attrs_ = attrs; }
    void set_attr(std::vector<attribute_t>&& attrs) { attrs_ = std::move(attrs);
    }

    bool find_attr(attribute_t attr) const {
        return std::find(attrs_.begin(), attrs_.end(), attr) !=
               std::end(attrs_);
    }

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

    bool is_default_constructible() const {
        return default_construct_ != nullptr;
    }
    any default_construct() const { return is_default_constructible() ? default_construct_() : any{}; }

protected:
    std::string name_;
    default_construct_fn default_construct_;

private:
    value_kind kind_;
    std::vector<attribute_t> attrs_;
};

#define SET_VALUE_CHECK(a, type)                    \
    ((a.access_type() == any::access_type::Ref ||   \
      a.access_type() == any::access_type::Copy) && \
     a.type_info()->kind() == type)

#define COPY_VALUE_CHECK(a, type) \
    (a.access_type() != any::access_type::Null && a.type_info()->kind() == type)

}  // namespace mirrow::drefl
