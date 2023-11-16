#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/type.hpp"
#include <string>
#include <vector>


namespace mirrow::drefl {

class enum_info;

class enum_item final {
public:
    using enum_numeric_type = long;

    template <typename T>
    enum_item(const std::string& name, T value,
              const class enum_info* enumerate)
        : enum_info_(enumerate),
          name_(name),
          value_(static_cast<enum_numeric_type>(value)) {}

    auto& name() const noexcept { return name_; }

    long value() const noexcept { return value_; }

    const enum_info* enum_info() const noexcept { return enum_info_; }

private:
    const class enum_info* enum_info_;
    std::string name_;
    enum_numeric_type value_;
};

class enum_info : public type {
public:
    template <typename>
    friend class enum_factory;

    using enum_numeric_type = typename enum_item::enum_numeric_type;

    enum_info() : type(value_kind::Enum) {}

    explicit enum_info(const std::string& name)
        : type(value_kind::Enum, name) {}

    long get_value(const any& elem) const { return *(long*)(elem.payload()); }

    void set_value(any& elem, enum_numeric_type value) const {
        if (!SET_VALUE_CHECK(elem, value_kind::Enum)) {
            MIRROW_LOG("can't set enum value to any");
            return;
        }

        *(long*)(elem.payload()) = value;
    }

    auto& enums() const noexcept { return items_; }

private:
    std::vector<enum_item> items_;

    template <typename T>
    void add(const std::string& name, T value) {
        items_.emplace_back(name, value, this);
    }
};

}  // namespace mirrow::drefl