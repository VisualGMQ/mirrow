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

toml::table serialize_class(const any& value);
toml::array serialize_array(const any& value);

class serialize_class_visitor : public class_visitor {
public:
    serialize_class_visitor(toml::table& tbl, const any& value)
        : tbl_(tbl), value_(value) {}

    void operator()(numeric_property& prop) {
        tbl_.emplace(prop.name(), serialize_numeric(prop.call_const(value_)));
    }

    void operator()(enum_property& prop) {
        tbl_.emplace(prop.name(), serialize_numeric(prop.call_const(value_)));
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

private:
    toml::table& tbl_;
    const any& value_;
};

toml::table serialize_class(const any& value) {
    toml::table tbl;

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
            case drefl::value_kind::None:
                MIRROW_LOG("unknown type, can't serialize");
                break;
            case value_kind::Array:
                arr.push_back(serialize_array(elem));
                break;
            case drefl::value_kind::Boolean:
                arr.push_back(serialize_boolean(elem));
                break;
            case drefl::value_kind::Numeric:
                arr.push_back(serialize_numeric(elem));
                break;
            case drefl::value_kind::String:
                arr.push_back(serialize_string(elem));
                break;
            case drefl::value_kind::Enum:
                arr.push_back(serialize_enum(elem));
                break;
            case drefl::value_kind::Class:
                arr.push_back(serialize_class(elem));
                break;
            case drefl::value_kind::Property:
            case drefl::value_kind::Pointer:
                MIRROW_LOG("can't serialize raw property/pointer");
                break;
        }
    }

    return arr;

}

void do_serialize(const any& value, toml::table& tbl, std::string_view name) {
    auto serialize_method = serialize_method_storage::instance().get_serialize(value.type_info());
    if (serialize_method) {
        serialize_method(tbl, value);
        return;
    }

    switch (value.type_info()->kind()) {
        case drefl::value_kind::None:
            MIRROW_LOG("unknown type, can't serialize");
            break;
        case drefl::value_kind::Boolean:
            tbl.emplace(name, serialize_boolean(value));
            break;
        case drefl::value_kind::Numeric:
            tbl.emplace(name, serialize_numeric(value));
            break;
        case drefl::value_kind::String:
            tbl.emplace(name, serialize_string(value));
            break;
        case drefl::value_kind::Enum:
            tbl.emplace(name, serialize_enum(value));
            break;
        case drefl::value_kind::Class:
            tbl.emplace(name, serialize_class(value));
            break;
        case drefl::value_kind::Array:
            tbl.emplace(name, serialize_array(value));
            break;
        case drefl::value_kind::Property:
            MIRROW_LOG("can't serialize property directly");
            break;
        case drefl::value_kind::Pointer:
            MIRROW_LOG("can't serialize pointer");
            break;
    }
}

toml::table serialize(const any& value, std::string_view name) {
    auto type = value.type_info();
    if (type->kind() == value_kind::Pointer ||
        type->kind() == value_kind::Property) {
        MIRROW_LOG(
            "How can I serialize a pointer or property? I can't do this!");
        return {};
    }

    toml::table tbl;
    do_serialize(value, tbl, name);
    return tbl;
}

}  // namespace mirrow::serd::drefl