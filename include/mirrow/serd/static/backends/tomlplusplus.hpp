#include "mirrow/serd/static/backends/toml.hpp"

#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mirrow {

namespace serd {

template <typename T>
constexpr bool should_serialize = std::is_class_v<T>;

template <typename T>
auto serialize(T&& value) {
    return std::forward<T>(value);
}

template <typename T, size_t N>
toml::array serialize(std::array<T, N>&& arr) {
    toml::array toml_arr;
    for (auto& value : arr) {
        toml_arr.emplace_back(serialize(value));
    }
    return toml_arr;
}

template <typename T>
toml::array serialize(std::vector<T>&& arr) {
    toml::array toml_arr;
    for (auto& value : arr) {
        toml_arr.emplace_back(serialize(value));
    }
    return toml_arr;
}

template <typename Key, typename Value>
toml::table serialize(std::unordered_map<Key, Value>&& tbl) {
    toml::table toml_tbl;
    for (auto& [key, value] : tbl) {
        toml_tbl.emplace(key, serialize(std::forward<Value>(value)));
    }
    return toml_tbl;
}

template <typename Key, typename Value>
toml::table serialize(std::unordered_set<Key, Value>&& tbl) {
    toml::table toml_tbl;
    for (auto& [key, value] : tbl) {
        toml_tbl.emplace(key, serialize(std::forward<Value>(value)));
    }
    return toml_tbl;
}

}  // namespace serd

}  // namespace mirrow