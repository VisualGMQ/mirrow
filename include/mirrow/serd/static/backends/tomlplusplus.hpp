#pragma once

#ifndef TOML_EXCEPTIONS
#define TOML_EXCEPTIONS 0
#endif
#include "toml++/toml.hpp"

#include "mirrow/assert.hpp"
#include "mirrow/serd/log.hpp"
#include "mirrow/srefl/reflect.hpp"
#include "mirrow/util/misc.hpp"

#include <array>
#include <cassert>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mirrow {

namespace serd {

namespace srefl {

// fwd
// template <typename T>
// auto serialize(const T&);

// some serialization
namespace impl {

// point out which toml type should serialize to

template <typename T, typename = void>
struct serialize_destination_type {
    using type = toml::table;
};

template <>
struct serialize_destination_type<std::string> {
    using type = toml::value<std::string>;
};

template <typename T>
struct serialize_destination_type<T, std::enable_if_t<std::is_same_v<T, bool>>> {
    using type = toml::value<bool>;
};

template <typename T>
struct serialize_destination_type<T, std::enable_if_t<!std::is_same_v<T, bool> && std::is_integral_v<T>>> {
    using type = toml::value<toml::int64_t>;
};

template <typename T>
struct serialize_destination_type<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    using type = toml::value<double>;
};

template <typename T>
struct serialize_destination_type<std::vector<T>> {
    using type = toml::array;
};

template <typename T>
struct serialize_destination_type<std::optional<T>> {
    using type =
        typename serialize_destination_type<util::remove_cvref_t<T>>::type;
};

template <typename T, size_t N>
struct serialize_destination_type<std::array<T, N>> {
    using type = toml::array;
};

template <typename T, typename U>
struct serialize_destination_type<std::unordered_map<T, U>> {
    using type = toml::table;
};

template <typename T>
struct serialize_destination_type<std::unordered_set<T>> {
    using type = toml::array;
};

// point out whether the type already has serialize_method

template <typename T, typename = void>
struct has_serialize_method {
    static constexpr bool value = false;
};

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<std::is_fundamental_v<T>>> {
    static constexpr bool value = true;
};

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<util::is_vector_v<T>>> {
    static constexpr bool value = true;
};

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<util::is_std_array_v<T>>> {
    static constexpr bool value = true;
};

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<util::is_unordered_map_v<T>>> {
    static constexpr bool value = true;
};

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<util::is_optional_v<T>>> {
    static constexpr bool value = true;
};

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<util::is_string_v<T>>> {
    static constexpr bool value = true;
};

}  // namespace impl

template <typename T>
using serialize_destination_type_t =
    typename impl::serialize_destination_type<T>::type;

template <typename T>
constexpr bool has_serialize_method_v = impl::has_serialize_method<T>::value;


// some SFINEA function to serialize specific type


/**
 * @brief serialize a data(the data must be reflected by static reflection)
 *
 * inner support types:
 * - fundamental type(integer, floating point, bool)
 * - std::vector
 * - std::array
 * - std::unordered_map
 * - std::unordered_set
 * - std::optional
 *
 * NOTE: we don't support std::set & set::map because data in TOML isn't in
 * order.
 * @tparam T  serialize type
 * @tparam U  serialize to the type
 */

template <typename T>
std::enable_if_t<!has_serialize_method_v<T>>
serialize(
    const T& value,
    serialize_destination_type_t<T>& tbl) {
    using type = util::remove_cvref_t<T>;

    ::mirrow::srefl::reflect_info<type> info = ::mirrow::srefl::reflect<type>();
    info.visit_member_variables([&tbl, &value](auto&& field) {
        auto& member = field.invoke(&value);

        if constexpr (util::is_optional_v<
                          util::remove_cvref_t<decltype(member)>>) {
            if (!member.has_value()) {
                return;
            }

            serialize_destination_type_t<typename util::remove_cvref_t<decltype(member.value())>> node;
            serialize(member, node);
            tbl.emplace(field.name(), node);
        } else {
            serialize_destination_type_t<util::remove_cvref_t<decltype(member)>> node;
            serialize(member, node);
            tbl.emplace(field.name(), node);
        }
    });
}

template <typename T>
std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>>
serialize(const T& value, serialize_destination_type_t<T>& node) {
    node.as_integer()->get() = static_cast<toml::int64_t>(value);
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>> serialize(
    const T& value, serialize_destination_type_t<T>& node) {
    node.as_floating_point()->get() = static_cast<double>(value);
}

template <typename T>
std::enable_if_t<std::is_same_v<bool, T>> serialize(
    const T& value, serialize_destination_type_t<T>& node) {
    node.as_boolean()->get() = value;
}

template <typename T>
std::enable_if_t<std::is_same_v<std::string, T>> serialize(
    const T& value, serialize_destination_type_t<T>& node) {
    node.as_string()->get() = value;
}

template <typename T>
std::enable_if_t<util::is_optional_v<T>> serialize(
    const T& value, serialize_destination_type_t<T>& node) {
    if (value.has_value()) {
        serialize<util::remove_cvref_t<typename T::value_type>>(
            value.value(), node);
    }
}

template <typename T>
std::enable_if_t<util::is_unordered_map_v<T>> serialize(
    const T& map, serialize_destination_type_t<T>& tbl) {
    using mapped_type = util::remove_cvref_t<typename T::mapped_type>;

    static_assert(util::is_string_v<typename T::key_type>,
                  "the key of unordered_map must std::string");

    for (auto& [key, value] : map) {
        using toml_type = serialize_destination_type_t<mapped_type>;
        toml_type new_node;
        serialize<typename T::key_type>(value, new_node);
        tbl.emplace(key, new_node);
    }
}

template <typename T>
std::enable_if_t<util::is_unordered_set_v<T>> serialize(
    const T& map, serialize_destination_type_t<T>& arr) {
    using value_type = util::remove_cvref_t<typename T::value_type>;

    for (auto& elem : arr) {
        using toml_type = serialize_destination_type_t<value_type>;
        toml_type new_node;
        serialize<value_type>(elem, new_node);
        arr.push_back(new_node);
    }
}

template <typename T>
std::enable_if_t<util::is_vector_v<T>> serialize(
    const T& elems, serialize_destination_type_t<T>& arr) {
    MIRROW_ASSERT(arr.is_array(), "des must array when serialize std::vector");

    for (auto& elem : elems) {
        using toml_type = serialize_destination_type_t<
            util::remove_cvref_t<typename T::value_type>>;
        toml_type new_node;
        serialize<typename T::value_type>(elem, new_node);
        arr.push_back(std::move(new_node));
    }
}

template <typename T>
std::enable_if_t<util::is_std_array_v<T>> serialize(
    const T& elems, serialize_destination_type_t<T>& arr) {
    MIRROW_ASSERT(arr.is_array(), "des must array when serialize std::vector");

    for (int i = 0; i < elems.size(); i++) {
        using toml_type = serialize_destination_type_t<
            util::remove_cvref_t<typename T::value_type>>;
        toml_type new_node;
        serialize(arr[i], new_node);
        arr.push_back(std::move(new_node));
    }
}


/******************************deserialize********************************************/

// functions for deserialize specific type

template <typename T>
std::enable_if_t<std::is_fundamental_v<T> && !std::is_same_v<T, bool>>
deserialize(const toml::node& node, T& elem) {
    if (!node.is_integer() && !node.is_floating_point()) {
        MIRROW_LOG(
            "deserialize numeric type require TOML node is numeric type");
        return;
    }

    if (node.is_integer()) {
        elem = static_cast<T>(node.as_integer()->get());
    } else if (node.is_floating_point()) {
        elem = static_cast<T>(node.as_floating_point()->get());
    }
}

template <typename T>
std::enable_if_t<std::is_same_v<T, bool>> deserialize(
    const toml::node& node, T& elem) {
    if (!node.is_boolean()) {
        MIRROW_LOG("deserialize bool type require TOML node is boolean type");
        return;
    }

    elem = node.as_boolean()->get();
}

template <typename T>
std::enable_if_t<util::is_string_v<T>> deserialize(const toml::node& node,
                                                        T& elem) {
    if (!node.is_string()) {
        MIRROW_LOG("deserialize string type require TOML node is string type");
        return;
    }

    elem = node.as_string()->get();
}

template <typename T>
std::enable_if_t<util::is_vector_v<T>> deserialize(const toml::node& node,
                                                        T& elems) {
    if (!node.is_array()) {
        MIRROW_LOG("deserialize std::vector require TOML node is array type");
        return;
    }

    for (auto& value : *node.as_array()) {
        auto& elem = elems.emplace_back();
        deserialize<typename T::value_type>(value, elem);
    }
}

template <typename T>
std::enable_if_t<util::is_std_array_v<T>> deserialize(
    const toml::node& node, T& elems) {
    if (!node.is_array()) {
        MIRROW_LOG("deserialize std::array require TOML node is array type");
        return;
    }

    auto& arr = *node.as_array();

    size_t suitable_size = std::min(elems.size(), arr.size());
    for (int i = 0; i < suitable_size; i++) {
        deserialize<T>(arr[i], elems[i]);
    }
}

template <typename T>
std::enable_if_t<util::is_unordered_map_v<T>> deserialize(
    const toml::node& node, T& elems) {
    if (!node.is_table()) {
        MIRROW_LOG(
            "deserialize std::unordered_map require TOML node is table type");
        return;
    }

    auto& tbl = *node.as_table();
    for (auto& [name, value] : tbl) {
        elems.emplace(name, deserialize<typename T::mapped_type>(value));
    }
}

template <typename T>
std::enable_if_t<util::is_unordered_set_v<T>> deserialize(
    const toml::node& node, T& elems) {
    if (!node.is_table()) {
        MIRROW_LOG(
            "deserialize std::unordered_set require TOML node is array type");
        return;
    }

    auto& arr = *node.as_array();
    for (auto& value : arr) {
        arr.insert(deserialize<typename T::value_type>(value));
    }
}

template <typename T>
std::enable_if_t<util::is_optional_v<T>> deserialize(
    const toml::node& node, T& instance) {
    typename T::value_type value;
    deserialize<typename T::value_type>(node, value);
    instance = value;
}

template <typename T>
std::enable_if_t<!has_serialize_method_v<T>> deserialize(
    const toml::node& node, T& instance) {
    if (!node.is_table()) {
        MIRROW_LOG("deserialize class require TOML node is table type");
        return;
    }

    auto& tbl = *node.as_table();

    auto type_info = ::mirrow::srefl::reflect<T>();

    type_info.visit_member_variables([&tbl, &instance](auto& field) {
        using type = typename util::remove_cvref_t<decltype(field)>::type;

        auto node = tbl[field.name()];
        auto& member = field.invoke(instance);
        if (node.node()) {
            deserialize<util::remove_cvref_t<decltype(member)>>(
                *node.node(), field.invoke(instance));
        }
    });
}

}  // namespace srefl

}  // namespace serd

}  // namespace mirrow