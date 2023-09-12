#pragma once

#ifndef TOML_EXCEPTIONS
#define TOML_EXCEPTIONS 0
#endif
#include "toml++/toml.hpp"

#include "mirrow/assert.hpp"
#include "mirrow/drefl/drefl.hpp"
#include "mirrow/drefl/factory.hpp"

#include <unordered_map>
#include <utility>

namespace mirrow {

namespace sred {

namespace drefl {

class serd_method_registry final {
public:
    using serial_type = std::function<void(toml::node&, std::string_view name,
                                           mirrow::drefl::any&)>;
    using deserial_type = std::function<mirrow::drefl::any(
        toml::node&, mirrow::drefl::type_info)>;
    using key_type = mirrow::drefl::type_info;
    using mapped_type = std::pair<serial_type, deserial_type>;
    using container_type = std::unordered_map<key_type, mapped_type>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    static auto& instance() {
        static serd_method_registry inst;
        return inst;
    }

    serd_method_registry() { pre_regist_default_methods(); }

    void regist(mirrow::drefl::type_info type, const serial_type& serial,
                const deserial_type& deserial) {
        datas_.insert_or_assign(type, std::make_pair(serial, deserial));
    }

    auto find(mirrow::drefl::type_info type) { return datas_.find(type); }

    const_iterator begin() const { return datas_.begin(); }

    const_iterator end() const { return datas_.end(); }

    iterator begin() { return datas_.begin(); }

    iterator end() { return datas_.end(); }

private:
    container_type datas_;

    static void default_numeric_serialize_method(toml::node& node,
                                                 std::string_view name,
                                                 mirrow::drefl::any& data) {
        auto type = data.type_info();
        if (!type.is_fundamental()) {
            MIRROW_LOG("can't serialize a non-numeric type");
            return;
        }

        auto fund = type.as_fundamental();
        if (fund.is_floating_pointer()) {
            double value = data.try_cast_floating_point().value();
            if (node.is_table()) {
                node.as_table()->emplace(name, value);
            } else if (node.is_array()) {
                node.as_array()->push_back(value);
            }
        } else {
            toml::int64_t value = 0;
            if (fund.is_signed()) {
                value = data.try_cast_integral().value();
            } else {
                value = data.try_cast_uintegral().value();
            }
            if (node.is_table()) {
                node.as_table()->emplace(name, value);
            } else if (node.is_array()) {
                node.as_array()->push_back(value);
            }
        }
    }

    static void default_string_serialize_method(toml::node& node,
                                                std::string_view name,
                                                mirrow::drefl::any& data) {
        auto type = data.type_info();
        MIRROW_ASSERT(type.is_class() && type.as_class().is_string(),
                      "can't serialize a non-string type");

        const std::string& str = data.cast<std::string>();
        if (node.is_array()) {
            node.as_array()->push_back(str);
        } else if (node.is_table()) {
            node.as_table()->emplace(name, str);
        }
    }

    static mirrow::drefl::any default_numeric_deserialize_method(
        toml::node& node, mirrow::drefl::type_info type) {
        if (node.is_integer()) {
            return mirrow::drefl::any{node.as_integer()->get()};
        } else if (node.is_floating_point()) {
            return mirrow::drefl::any{node.as_floating_point()->get()};
        } else {
            return mirrow::drefl::any{};
        }
    }

    static mirrow::drefl::any default_string_deserialize_method(
        toml::node& node, mirrow::drefl::type_info type) {
        MIRROW_ASSERT(type.is_class() && type.as_class().is_string(),
                      "can't deserialize a non-string type");

        if (node.is_string()) {
            return mirrow::drefl::any{node.as_string()->get()};
        } else {
            return mirrow::drefl::any{};
        }
    }

    void pre_regist_default_methods() {
        auto default_numeric_serd_method =
            std::make_pair(default_numeric_serialize_method,
                           default_numeric_deserialize_method);
        datas_[mirrow::drefl::reflected_type<char>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<short>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<int>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<long>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<long long>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<unsigned char>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<unsigned short>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<unsigned int>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<unsigned long>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<unsigned long long>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<float>()] =
            default_numeric_serd_method;
        datas_[mirrow::drefl::reflected_type<double>()] =
            default_numeric_serd_method;

        datas_[mirrow::drefl::reflected_type<std::string>()] = std::make_pair(
            default_string_serialize_method, default_string_deserialize_method);
    }
};

toml::table serialize_class(mirrow::drefl::any& data);

namespace internal {

void default_class_serialize_method(toml::node&, std::string_view,
                                    mirrow::drefl::any&);
void default_container_serialize_method(toml::node&, std::string_view,
                                        mirrow::drefl::any&);

void serialize_one_data(toml::node& node,
                        [[maybe_unused]] std::string_view name,
                        mirrow::drefl::any& data) {
    MIRROW_ASSERT(node.is_array() || node.is_table(),
                  "data only can be serialized into array or table");

    auto data_type = data.type_info();

    auto& serd_reg = serd_method_registry::instance();
    auto it = serd_reg.find(data_type);

    if (it != serd_reg.end()) {
        it->second.first(node, name, data);
        return;
    }

    if (data_type.is_array() ||
        (data_type.is_class() && data_type.as_class().is_container())) {
        toml::array sub_arr;
        default_container_serialize_method(sub_arr, name, data);
        if (node.is_array()) {
            node.as_array()->push_back(sub_arr);
        } else {
            node.as_table()->emplace(name, sub_arr);
        }
    } else if (data_type.is_class()) {
        toml::table sub_tbl;
        default_class_serialize_method(sub_tbl, name, data);
        if (node.is_array()) {
            node.as_array()->push_back(sub_tbl);
        } else {
            node.as_table()->emplace(name, sub_tbl);
        }
    } else {
        MIRROW_LOG("currently we can't serialize pointer type");
    }
}

inline void default_container_serialize_method(
    toml::node& node, [[maybe_unused]] std::string_view name,
    mirrow::drefl::any& data) {
    auto type = data.type_info();
    MIRROW_ASSERT(
        type.is_array() || (type.is_class() && type.as_class().is_container()),
        "can't serialize non-array/container type");
    MIRROW_ASSERT(node.is_array(),
                  "must serialize container type into toml::array");
    auto& arr = *node.as_array();

    data.travel_elements([&node, &arr, name](mirrow::drefl::any& elem) {
        serialize_one_data(node, name, elem);
    });
}

inline void default_class_serialize_method(
    toml::node& node, [[maybe_unused]] std::string_view name,
    mirrow::drefl::any& data) {
    auto type = data.type_info();

    MIRROW_ASSERT(node.is_table(),
                  "class can be only serialized into toml::table");
    MIRROW_ASSERT(type.is_class(), "can't serialize non-class type");

    toml::table& tbl = *node.as_table();

    auto& serd_reg = serd_method_registry::instance();

    for (auto& var : type.as_class().vars()) {
        auto result = mirrow::drefl::invoke_by_any(var, &data);
        serialize_one_data(tbl, var.name(), result);
    }
}

}  // namespace internal

toml::table serialize_class(mirrow::drefl::any& data) {
    auto type = data.type_info();

    MIRROW_ASSERT(type.is_class(), "can't serialize non-class type");

    auto& serd_reg = serd_method_registry::instance();
    auto it = serd_reg.find(type);

    toml::table result;

    if (it != serd_reg.end()) {
        it->second.first(result, "", data);
    } else {
        internal::default_class_serialize_method(result, "", data);
    }

    return result;
}

}  // namespace drefl

}  // namespace sred

}  // namespace mirrow
