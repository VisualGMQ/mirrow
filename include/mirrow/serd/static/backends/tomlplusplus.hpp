#include "toml++/toml.hpp"

#include "mirrow/srefl/reflect.hpp"

#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mirrow {

namespace serd {

// forward declaration

template <typename T>
auto serialize(T&& value);

template <typename T, size_t N>
toml::array serialize(std::array<T, N>&&);

template <typename T>
toml::array serialize(std::vector<T>&&);

template <typename Key, typename Value>
toml::table serialize(std::unordered_map<Key, Value>&&);

template <typename Key, typename Value>
toml::table serialize(std::unordered_set<Key, Value>&&);


// implementations

template <typename T>
auto serialize(T&& value) {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_fundamental_v<type> || std::is_same_v<type, std::string>) {
        return std::forward<T>(value);
    } else {
        ::mirrow::srefl::reflect_info<type> info = ::mirrow::srefl::reflect<type>();
        toml::table tbl;
        info.visit_member_variables([&tbl, &value](auto&& field){
            tbl.emplace(field.name(), serialize(field.invoke(&value)));
        });
        return tbl;
    }
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