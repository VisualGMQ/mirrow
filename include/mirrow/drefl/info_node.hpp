#pragma once

#include <string_view>

namespace mirrow {

namespace drefl {

namespace internal {

struct type_node;

struct ctor_node final {
    type_node* parent = nullptr;
    ctor_node* next = nullptr;  // the next ctor
};

struct dtor_node final {
    type_node* next = nullptr;  // the next dtor
};

struct type_node final {
    enum class type {
        Class,
        Function,
        Variable,
    };

    enum class scope {
        Member,
        Global,
    };

    type_node* next = nullptr; // the next type_node

    std::string_view name;

    type type;
    scope scope;
    bool is_const;
    bool is_reference;

    const ctor_node* ctor = nullptr;  // a link list, store all constructors from one type
    const dtor_node* dtor = nullptr;  // a link list, store all destructors from one type
};

template <typename T>
constexpr enum type_node::type get_node_type() {
    if constexpr (std::is_class_v<T>) {
        return type_node::type::Class;
    } else if constexpr (std::is_function_v<T>) {
        return type_node::type::Function;
    } else {
        return type_node::type::Variable;
    }
}

template <typename T>
constexpr enum type_node::scope get_node_scope() {
    if constexpr (std::is_member_pointer_v<T>) {
        return type_node::scope::Member;
    } else {
        return type_node::scope::Global;
    }
}

template <typename T>
struct info_node final {
    inline static type_node* type = nullptr;

    template <typename ArgList>
    static constexpr ctor_node* ctor = nullptr;

    inline static type_node* resolve() {
        static type_node node = {
            nullptr,
            "",
            internal::get_node_type<T>(),
            internal::get_node_scope<T>(),
            std::is_const_v<T>,
            std::is_reference_v<T>,
        };
        if (!type) {
            type = &node;
        }
        return type;
    }
};

struct registry final {
    inline static type_node* root = nullptr;
};

}

}

}