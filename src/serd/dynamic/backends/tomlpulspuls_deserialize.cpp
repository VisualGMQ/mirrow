#include "mirrow/drefl/array.hpp"
#include "mirrow/drefl/class_visitor.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/serd/dynamic/backends/tomlplusplus.hpp"

#define TOML_IMPLEMENTATION
#include "toml++/toml.hpp"

namespace mirrow::serd::drefl {

void deserialize_numeric(any& obj, const toml::node& node) {
    MIRROW_ASSERT(node.is_number(),
                  "can't deserialize non-number node to numeric type");

    double value = 0;
    if (node.is_integer()) {
        value = node.as_integer()->get();
    } else {
        value = node.as_floating_point()->get();
    }
    obj.type_info()->as_numeric()->set_value(obj, value);
}

void deserialize_boolean(any& obj, const toml::node& node) {
    MIRROW_ASSERT(node.is_boolean(),
                  "can't deserialize non-bool node to boolean type");

    obj.type_info()->as_boolean()->set_value(obj, node.as_boolean()->get());
}

void deserialize_string(any& obj, const toml::node& node) {
    MIRROW_ASSERT(node.is_string(),
                  "can't deserialize non-bool node to boolean type");

    obj.type_info()->as_string()->set_value(obj, node.as_string()->get());
}

void deserialize_enum(any& obj, const toml::node& node) {
    MIRROW_ASSERT(node.is_integer(),
                  "can't deserialize non-integer node to enum type");

    obj.type_info()->as_enum()->set_value(obj, node.as_integer()->get());
}

void deserialize_class(any& obj, const toml::node& node);

class class_deserialize_visitor : public class_visitor {
public:
    class_deserialize_visitor(any& obj, const toml::table& tbl)
        : obj_(obj), tbl_(tbl) {}

    void operator()(numeric_property& prop) {
        if (tbl_.contains(prop.name())) {
            auto value = prop.call(obj_);
            deserialize_numeric(value, *tbl_.get(prop.name()));
        } else {
            MIRROW_LOG("property " + prop.name() + " don't exists, ignore");
        }
    }

    void operator()(enum_property& prop) {
        if (tbl_.contains(prop.name())) {
            auto value = prop.call(obj_);
            deserialize_enum(value, *tbl_.get(prop.name()));
        } else {
            MIRROW_LOG("property " + prop.name() + " don't exists, ignore");
        }
    }

    void operator()(clazz_property& prop) {
        if (tbl_.contains(prop.name())) {
            auto value = prop.call(obj_);
            deserialize_class(value, *tbl_.get(prop.name()));
        } else {
            MIRROW_LOG("property " + prop.name() + " don't exists, ignore");
        }
    }

    void operator()(string_property& prop) {
        if (tbl_.contains(prop.name())) {
            auto value = prop.call(obj_);
            deserialize_string(value, *tbl_.get(prop.name()));
        } else {
            MIRROW_LOG("property " + prop.name() + " don't exists, ignore");
        }
    }

    void operator()(boolean_property& prop) {
        if (tbl_.contains(prop.name())) {
            auto value = prop.call(obj_);
            deserialize_boolean(value, *tbl_.get(prop.name()));
        } else {
            MIRROW_LOG("property " + prop.name() + " don't exists, ignore");
        }
    }

    void operator()(pointer_property& prop) {
        MIRROW_LOG("can't deserialize pointer property " + prop.name());
    }

    void operator()(array_property& prop) {

    }

private:
    any& obj_;
    const toml::table& tbl_;
};

void deserialize_class(any& obj, const toml::node& node) {
    MIRROW_ASSERT(node.is_table(),
                  "can't deserialize non-table node to class type");

    auto& tbl = *node.as_table();
    auto& clazz = *obj.type_info()->as_class();

    class_deserialize_visitor visitor{obj, tbl};

    for (auto& prop : clazz.properties()) {
        prop->visit(&visitor);
    }
}

void deserialize_array(any& obj, const toml::node& node) {
    MIRROW_ASSERT(node.is_array(),
                  "can't deserialize non-array node to array type");

    auto& arr = *node.as_array();

    size_t size = arr.size();
    auto arr_type = obj.type_info()->as_array();
    if (arr_type->array_type() == array::array_type::Static) {
        size = std::min(arr_type->size(obj), arr.size());
    }

    for (int i = 0; i < size; i++) {
        auto node = arr.get(i);
        mirrow::drefl::any elem;
        switch (arr_type->elem_type()->kind()) {
            case drefl::value_kind::None:
                MIRROW_LOG("unknown type, can't deserialize");
                break;
            case drefl::value_kind::Boolean:
                deserialize_boolean(elem, *node);
                arr_type->push_back(elem, obj);
                break;
            case drefl::value_kind::Numeric:
                deserialize_numeric(elem, *node);
                arr_type->push_back(elem, obj);
                break;
            case drefl::value_kind::String:
                deserialize_string(elem, *node);
                arr_type->push_back(elem, obj);
                break;
            case drefl::value_kind::Enum:
                deserialize_enum(elem, *node);
                arr_type->push_back(elem, obj);
                break;
            case drefl::value_kind::Class:
                deserialize_class(elem, *node);
                arr_type->push_back(elem, obj);
                break;
            case drefl::value_kind::Array:
                deserialize_array(elem, *node);
                arr_type->push_back(elem, obj);
                break;
            case drefl::value_kind::Property:
            case drefl::value_kind::Pointer:
                MIRROW_LOG("can't deserialize property/pointer");
                break;
        }
    }
}

void do_deserialize(any& obj, const toml::node& node) {
    if (auto func = serialize_method_storage::instance().get_deserialize(obj.type_info()); func) {
        func(node, obj);
        return;
    }

    switch (obj.type_info()->kind()) {
        case drefl::value_kind::Boolean:
            deserialize_boolean(obj, node);
            break;
        case drefl::value_kind::Numeric:
            deserialize_numeric(obj, node);
            break;
        case drefl::value_kind::String:
            deserialize_string(obj, node);
            break;
        case drefl::value_kind::Enum:
            deserialize_enum(obj, node);
            break;
        case drefl::value_kind::Class:
            deserialize_class(obj, *node.as_table()->get(obj.type_info()->name()));
            break;
        case drefl::value_kind::Array:
            deserialize_array(obj, node);
        default:;
    }
}

void deserialize(any& obj, const toml::node& node) {
    auto typeinfo = obj.type_info();
    switch (typeinfo->kind()) {
        case drefl::value_kind::None:
            MIRROW_LOG("unknown type");
            break;
        case drefl::value_kind::Boolean:
        case drefl::value_kind::Numeric:
        case drefl::value_kind::String:
        case drefl::value_kind::Enum:
        case drefl::value_kind::Class:
        case drefl::value_kind::Array:
            do_deserialize(obj, node);
            break;
        case drefl::value_kind::Property:
        case drefl::value_kind::Pointer:
            MIRROW_LOG("don't known how to deserialize property/pointer");
            break;
    }
}

}  // namespace mirrow::serd::drefl