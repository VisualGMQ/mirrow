#pragma once

#include "mirrow/util/misc.hpp"

#include <string_view>
#include <string>
#include <vector>

namespace mirrow {

namespace drefl {

class any;

namespace internal {

struct type_node;

struct ctor_node final {
    type_node* parent = nullptr;
};

struct function_node final {
    type_node* parent = nullptr;
    std::string name;
    bool is_member;
    bool is_const;
    any(* invoke)(any*) = nullptr;
};

struct variable_node final {
    type_node* parent = nullptr;
    std::string name;
    bool is_member;
    bool is_const;
    any(*invoke)(any*) = nullptr;
};

struct dtor_node final {
    type_node* parent = nullptr;
};

struct type_node final {
    enum class type {
        Class,
        Function,
        Variable,
        Enum,
    };

    type_node* next = nullptr;  // the next type_node

    std::string name;

    type type;

    std::vector<ctor_node*> ctors;
    std::vector<dtor_node*> dtors;
    std::vector<function_node*> funcs;
    std::vector<variable_node*> vars;
};

template <typename T>
constexpr enum type_node::type get_node_type() {
    using type = util::remove_cvref_t<T>;

    if constexpr (std::is_class_v<type>) {
        return type_node::type::Class;
    } else if constexpr (std::is_function_v<type> ||
                         std::is_member_function_pointer_v<type>) {
        return type_node::type::Function;
    } else if constexpr (std::is_enum_v<type>) {
        return type_node::type::Enum;
    } else {
        return type_node::type::Variable;
    }
}

template <typename T>
struct info_node final {
    inline static type_node* type = nullptr;

    template <typename ArgList>
    inline static ctor_node* ctor = nullptr;

    template <auto>
    inline static function_node* func = nullptr;

    template <auto>
    inline static variable_node* var = nullptr;

    inline static type_node* resolve() {
        static type_node node = {
            nullptr,
            "",
            internal::get_node_type<T>(),
        };
        if (!type) {
            type = &node;
        }
        return type;
    }

    bool operator==(const info_node& node) const {
        return true;
    }

    bool operator!=(const info_node& node) const {
        return false;
    }
};

struct registry final {
    inline static type_node* root = nullptr;
};

}  // namespace internal

}  // namespace drefl

}  // namespace mirrow