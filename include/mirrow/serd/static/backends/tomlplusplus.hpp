#pragma once

#ifndef TOML_EXCEPTIONS
#define TOML_EXCEPTIONS 0
#endif
#include "toml++/toml.hpp"

#include "mirrow/serd/log.hpp"
#include "mirrow/srefl/reflect.hpp"
#include "mirrow/util/misc.hpp"
#include "mirrow/assert.hpp"

#include <array>
#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mirrow {

namespace serd {

namespace srefl {

namespace internal {

template <typename T>
constexpr bool has_default_serial_method_v =
    util::is_vector_v<T> || util::is_unordered_map_v<T> || util::is_string_v<T> ||
    std::is_fundamental_v<T>;

template <typename T>
constexpr bool can_serial_directly = util::is_string_v<T> || std::is_fundamental_v<T>;

template <typename T>
constexpr bool has_default_deserial_method_v =
    util::is_vector_v<T> || util::is_unordered_map_v<T> || util::is_string_v<T> ||
    std::is_fundamental_v<T>;

template <typename T>
constexpr bool can_deserial_directly =
    util::is_string_v<T> || std::is_fundamental_v<T>;

}  // namespace internal

template <typename T>
auto serialize(const T&);

namespace impl {

// for fundamental & std::string serialize
template <typename T>
T serialize_directly(const T& value) {
    return value;
}

// for class serialize
template <typename T>
toml::table serialize_class(const T& value) {
    using type = util::remove_cvref_t<T>;
    ::mirrow::srefl::reflect_info<type> info = ::mirrow::srefl::reflect<type>();
    toml::table tbl;
    info.visit_member_variables([&tbl, &value](auto&& field) {
        tbl.emplace(field.name(), serialize(field.invoke(&value)));
    });
    return tbl;
}

template <typename T, size_t N>
toml::array serialize_array(const std::array<T, N>& arr) {
    toml::array toml_arr;
    for (auto& value : arr) {
        toml_arr.emplace_back(serialize<T>(value));
    }
    return toml_arr;
}

template <typename T>
toml::array serialize_vector(const std::vector<T>& arr) {
    toml::array toml_arr;
    for (auto& value : arr) {
        toml_arr.emplace_back(serialize<T>(value));
    }
    return toml_arr;
}

template <typename Key, typename Value>
toml::table serialize_umap(const std::unordered_map<Key, Value>& tbl) {
    toml::table toml_tbl;
    for (auto& [key, value] : tbl) {
        toml_tbl.emplace(key, serialize<Value>(std::forward<Value>(value)));
    }
    return toml_tbl;
}

template <typename Key, typename Value>
toml::table serialize_uset(const std::unordered_set<Key, Value>& tbl) {
    toml::table toml_tbl;
    for (auto& [key, value] : tbl) {
        toml_tbl.emplace(key, serialize<Value>(std::forward<Value>(value)));
    }
    return toml_tbl;
}

}  // namespace impl

/**
 * @brief serialize a type
 */
template <typename T>
auto serialize(const T& value) {
    using type = util::remove_cvref_t<T>;

    if constexpr (internal::can_serial_directly<T>) {
        return impl::serialize_directly(value);
    } else if constexpr (util::is_vector_v<T>) {
        return impl::serialize_vector(value);
    } else if constexpr (util::is_unordered_map_v<T>) {
        return impl::serialize_umap(value);
    } else {
        return impl::serialize_class(value);
    }
}

template <typename T>
T deserialize(const toml::node&);

template <typename T>
T deserialize(const toml::table&);

namespace impl {

template <typename T>
std::vector<T> deserialize_array_table(const toml::array& arr) {
    std::vector<T> result;
    for (auto& value : arr) {
        result.push_back(deserialize<T>(value));
    }
    return result;
}

template <typename T>
std::vector<T> deserialize_array(const toml::array& arr) {
    std::vector<T> result;
    for (auto& value : arr) {
        result.push_back(deserialize<T>(value));
    }
    return result;
}

template <typename Key, typename Value>
std::unordered_map<Key, Value> deserialize_umap(const toml::table& tbl) {
    std::unordered_map<Key, Value> result;
    for (auto& [name, value] : tbl) {
        result.emplace(name, deserialize<Value>(value));
    }
    return result;
}

}  // namespace impl

template <typename T>
struct show_tmpl;

template <typename T>
T deserialize(const toml::table& tbl) {
    static_assert(std::is_same_v<util::remove_cvref_t<T>, T>, "T can't has qualifier/reference");

    T instance;

    auto type_info = ::mirrow::srefl::reflect<T>();

    type_info.visit_member_variables([&tbl, &instance](auto& field) {
        using type = typename util::remove_cvref_t<decltype(field)>::type;

        if constexpr (std::is_floating_point_v<type> || std::is_integral_v<type>) {
            auto node = tbl[field.name()];
            if (node.is_integer() || node.is_floating_point()) {
                if (node.is_floating_point()) {
                    field.invoke(instance) = static_cast<type>(node.as_floating_point()->get());
                } else {
                    field.invoke(instance) = static_cast<type>(node.as_integer()->get());
                }
            } else if (node.is_boolean()) {
                field.invoke(instance) = static_cast<type>(node.as_boolean()->get());
            } else {
                LOG("node type is not numeric");
            }
        } else if constexpr (util::is_string_v<type>) {
            auto node = tbl[field.name()];
            if (node.is_string()) {
                field.invoke(instance) = node.as_string()->get();
            } else {
                LOG("node type is not string");
            }
        } else if constexpr (util::is_vector_v<type>) {
            auto node = tbl[field.name()];
            if (node.is_array()) {
                field.invoke(instance) =
                    impl::deserialize_array<typename type::value_type>(
                        *node.as_array());
            } else {
                LOG("node type is not string");
            }
        } else if constexpr (util::is_unordered_map_v<type>) {
            auto node = tbl[field.name()];
            if (node.is_table()) {
                field.invoke(instance) =
                    impl::deserialize_umap<typename type::key_type,
                                           typename type::mapped_type>(
                        *tbl[field.name()].as_table());
            } else {
                LOG("node type is not table");
            }
        } else if constexpr (std::is_class_v<type>) {
            field.invoke(instance) =
                deserialize<type>(*tbl[field.name()].as_table());
        } else {
            assert(("don't support type", false));
        }
    });

    return instance;
}

template <typename T>
T deserialize(const toml::node& node) {
    using type = util::remove_cvref_t<T>;

    if constexpr (std::is_floating_point_v<type>) {
        if (node.is_floating_point()) {
            return node.as_floating_point()->get();
        } else {
            LOG("node is not floating point");
        }
    } else if constexpr (std::is_integral_v<type>) {
        if (node.is_integer()) {
            return node.as_integer()->get();
        } else if (node.is_boolean()) {
            return node.as_boolean()->get();
        } else {
            LOG("node is not integral/boolean");
        }
    } else if constexpr (util::is_string_v<type>) {
        if (node.is_string()) {
            return node.as_string()->get();
        } else {
            LOG("node is not string");
        }
    } else if constexpr (util::is_vector_v<type>) {
        if (node.is_array()) {
            return impl::deserialize_array<type>(*node.as_array());
        } else {
            LOG("node is not array");
        }
    } else if constexpr (util::is_unordered_map_v<type>) {
        if (node.is_table()) {
            return impl::deserialize_umap<typename type::key_type,
                                          typename type::mapped_type>(
                *node.as_table());
        } else {
            LOG("node is not table");
        }
    } else if constexpr (std::is_class_v<type>) {
        if (node.is_table()) {
            return deserialize<type>(*node.as_table());
        } else {
            LOG("node is not table");
        }
    } else {
        MIRROW_ASSERT(false, "don't support type");
    }
}

}  // namespace srefl

}  // namespace serd

}  // namespace mirrow