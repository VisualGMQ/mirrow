#include "mirrow/serd/dynamic/backends/tomlplusplus.hpp"
#include "mirrow/drefl/class_visitor.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/value_kind.hpp"

#define TOML_IMPLEMENTATION
#include "toml++/toml.hpp"

#include <string_view>


namespace mirrow::serd::drefl {

double serialize_numeric(const any& value) {
    auto& numeric = *value.type_info()->as_numeric();
    return numeric.get_value(value);
}

bool serialize_boolean(const any& value) {
    return value.type_info()->as_boolean()->get_value(value);
}

std::string_view serialize_string(const any& value) {
    return value.type_info()->as_string()->get_str_view(value);
}

long serialize_enum(const any& value) {
    return value.type_info()->as_enum()->get_value(value);
}

void serialize_optional(const any& value, std::string_view name,
                        toml::node& node);

toml::array serialize_array(const any& value);

class serialize_class_visitor : public class_visitor {
public:
    serialize_class_visitor(toml::table& tbl, const any& value)
        : tbl_(tbl), value_(value) {}

    void operator()(numeric_property& prop) {
        tbl_.emplace(prop.name(), serialize_numeric(prop.call_const(value_)));
    }

    void operator()(enum_property& prop) {
        tbl_.emplace(prop.name(), serialize_enum(prop.call_const(value_)));
    }

    void operator()(clazz_property& prop) {
        tbl_.emplace(prop.name(), serialize_class(prop.call_const(value_)));
    }

    void operator()(string_property& prop) {
        tbl_.emplace(prop.name(), serialize_string(prop.call_const(value_)));
    }

    void operator()(boolean_property& prop) {
        tbl_.emplace(prop.name(), serialize_boolean(prop.call_const(value_)));
    }

    void operator()(pointer_property& prop) {
        MIRROW_LOG("pointer property can't be serialize");
    }

    void operator()(array_property& prop) {
        tbl_.emplace(prop.name(), serialize_array(prop.call_const(value_)));
    }

    void operator()(optional_property& prop) {
        auto value = prop.call_const(value_);
        if (value.type_info()->as_optional()->has_value(value)) {
            serialize_optional(value, prop.name(), tbl_);
        }
    }

private:
    toml::table& tbl_;
    const any& value_;
};

toml::table serialize_class(const any& value) {
    toml::table tbl;

    if (auto f = serialize_method_storage::instance().get_serialize(
            value.type_info());
        f) {
        f(tbl, value);
        return tbl;
    }

    auto& clazz = *value.type_info()->as_class();
    serialize_class_visitor visitor{tbl, value};
    for (auto& prop : clazz.properties()) {
        prop->visit(&visitor);
    }

    return tbl;
}

toml::array serialize_array(const any& value) {
    // IMPROVE: use iterator to improve effect when value is std::list
    toml::array arr;

    auto arr_type = value.type_info()->as_array();
    for (int i = 0; i < arr_type->size(value); i++) {
        auto elem = value.type_info()->as_array()->get_const(i, value);
        auto type = elem.type_info();
        switch (type->kind()) {
            case value_kind::None:
                MIRROW_LOG("unknown type, can't serialize");
                break;
            case value_kind::Array:
                arr.push_back(serialize_array(elem));
                break;
            case value_kind::Boolean:
                arr.push_back(serialize_boolean(elem));
                break;
            case value_kind::Numeric:
                arr.push_back(serialize_numeric(elem));
                break;
            case value_kind::String:
                arr.push_back(serialize_string(elem));
                break;
            case value_kind::Enum:
                arr.push_back(serialize_enum(elem));
                break;
            case value_kind::Class:
                arr.push_back(serialize_class(elem));
                break;
            case value_kind::Optional:
                serialize_optional(elem, "", arr);
                break;
            case value_kind::Property:
            case value_kind::Pointer:
                MIRROW_LOG("can't serialize raw property/pointer");
                break;
        }
    }

    return arr;
}

void do_serialize(const any& value, toml::table& tbl, std::string_view name) {
    auto serialize_method =
        serialize_method_storage::instance().get_serialize(value.type_info());
    if (serialize_method) {
        serialize_method(tbl, value);
        return;
    }

    switch (value.type_info()->kind()) {
        case value_kind::None:
            MIRROW_LOG("unknown type, can't serialize");
            break;
        case value_kind::Boolean:
            tbl.emplace(name, serialize_boolean(value));
            break;
        case value_kind::Numeric:
            tbl.emplace(name, serialize_numeric(value));
            break;
        case value_kind::String:
            tbl.emplace(name, serialize_string(value));
            break;
        case value_kind::Enum:
            tbl.emplace(name, serialize_enum(value));
            break;
        case value_kind::Class:
            tbl.emplace(name, serialize_class(value));
            break;
        case value_kind::Array:
            tbl.emplace(name, serialize_array(value));
            break;
        case value_kind::Optional:
            serialize_optional(value, name, tbl);
            break;
        case value_kind::Property:
            MIRROW_LOG("can't serialize property directly");
            break;
        case value_kind::Pointer:
            MIRROW_LOG("can't serialize pointer");
            break;
    }
}

void serialize(toml::table& tbl, const any& value, std::string_view name) {
    auto type = value.type_info();
    if (type->kind() == value_kind::Pointer ||
        type->kind() == value_kind::Property) {
        MIRROW_LOG(
            "How can I serialize a pointer or property? I can't do this!");
    }

    do_serialize(value, tbl, name);
}

void serialize_optional(const any& value, std::string_view name,
                        toml::node& node) {
    auto optional_type = value.type_info()->as_optional();

    if (!optional_type->has_value(value)) {
        return;
    }
    if (node.is_table()) {
        serialize(*node.as_table(), optional_type->get_value_const(value),
                  name);
    } else if (node.is_array()) {
        auto elem = optional_type->get_value_const(value);
        auto arr = *node.as_array();
        switch (elem.type_info()->kind()) {
            case value_kind::None:
                MIRROW_LOG("can't serialize unknown value");
                break;
            case value_kind::Boolean:
                arr.push_back(serialize_boolean(elem));
                break;
            case value_kind::Numeric:
                arr.push_back(serialize_numeric(elem));
                break;
            case value_kind::String:
                arr.push_back(serialize_string(elem));
                break;
            case value_kind::Enum:
                arr.push_back(serialize_enum(elem));
                break;
            case value_kind::Class:
                arr.push_back(serialize_class(elem));
                break;
            case value_kind::Array:
                arr.push_back(serialize_array(elem));
                break;
            case value_kind::Property:
            case value_kind::Pointer:
            case value_kind::Optional:
                MIRROW_LOG(
                    "can't serialize property/pointer/optional<optional<>>");
                break;
        }
    }
}

}  // namespace mirrow::serd::drefl