#pragma once

#ifndef TOML_EXCEPTIONS
#define TOML_EXCEPTIONS 0
#endif
#include "toml++/toml.hpp"

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/factory.hpp"

#include <functional>
#include <unordered_map>
#include <utility>


namespace mirrow::serd::drefl {

using namespace ::mirrow::drefl;

class serialize_method_storage {
public:
    using serialize_fn = std::function<void(toml::node&, const any&)>;
    using deserialize_fn = std::function<void(const toml::node&, any&)>;

    struct serd_methods {
        serialize_fn serialize = nullptr;
        deserialize_fn deserialize = nullptr;
    };

    static auto& instance() {
        static serialize_method_storage instance;
        return instance;
    }

    void regist_serialize(const type* type, const serialize_fn& f) {
        if (auto it = methods_.find(type); it != methods_.end()) {
            it->second.serialize = f;
        } else {
            methods_.insert_or_assign(type, serd_methods{f, nullptr});
        }
    }

    void regist_deserialize(const type* type, const deserialize_fn& f) {
        if (auto it = methods_.find(type); it != methods_.end()) {
            it->second.deserialize = f;
        } else {
            methods_.insert_or_assign(type, serd_methods{nullptr, f});
        }
    }

    serialize_fn get_serialize(const type* type) {
        if (auto it = methods_.find(type); it != methods_.end()) {
            return it->second.serialize;
        }
        return nullptr;
    }

    deserialize_fn get_deserialize(const type* type) {
        if (auto it = methods_.find(type); it != methods_.end()) {
            return it->second.deserialize;
        }
        return nullptr;
    }

private:
    std::unordered_map<const type*, serd_methods> methods_;
};

toml::table serialize(const any& value, std::string_view name);
void deserialize(any& obj, const toml::node& node);

}  // namespace mirrow::serd::drefl
