#pragma once

#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/misc.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace mirrow {

namespace drefl {

class any;

enum class type_category {
    Compound,        // a compound type(with qualifier/reference/pointer/array)
    Fundamental,     // fundamental type(without pointer)
    Class,           // class type
    Function,        // function type: Ret(Param...)
    MemberFunction,  // member function type: Ret(Class::*)(Param...)
    MemberObject,    // member variable type: Ret Class::*
    Enum,            // enumeration
};

namespace internal {

struct type_node;

struct ctor_node final {
    type_node* parent = nullptr;
};

struct function_node final {
    type_node* parent = nullptr;
    type_node* type = nullptr;
    std::string name;
    bool is_const_member;
    any (*invoke)(any*) = nullptr;
};

struct variable_node final {
    type_node* parent = nullptr;
    type_node* type = nullptr;
    std::string name;
    any (*invoke)(any*) = nullptr;
};

struct dtor_node final {
    type_node* parent = nullptr;
};

struct type_node final {
    type_category category;  // type category

    std::string name;

    bool is_member_pointer;
    bool is_pointer;
    bool is_reference;
    bool is_const;
    bool is_volatile;
    bool is_array;

    bool is_integral;
    bool is_floating_pointer;
    bool is_signed;
    bool is_string;
    bool is_container;

    bool is_const_member;

    type_node* raw_type;  // type after remove pointer/reference/const/volatile
                          // qualifier:

    // datas for class type
    std::vector<ctor_node*> ctors;
    std::vector<dtor_node*> dtors;
    std::vector<function_node*> funcs;
    std::vector<variable_node*> vars;
};

template <typename T>
constexpr type_category get_node_category() {
    if constexpr (!std::is_same_v<util::completely_strip_type_t<T>, T>) {
        return type_category::Compound;
    } else if constexpr (std::is_class_v<T>) {
        return type_category::Class;
    } else if constexpr (std::is_function_v<T>) {
        return type_category::Function;
    } else if constexpr (std::is_member_object_pointer_v<T>) {
        return type_category::MemberObject;
    } else if constexpr (std::is_member_function_pointer_v<T>) {
        return type_category::MemberFunction;
    } else if constexpr (std::is_enum_v<T>) {
        return type_category::Enum;
    } else {
        return type_category::Fundamental;
    }
}

template <typename T>
std::string get_fundamental_type_name() {
    static_assert(std::is_fundamental_v<T>, "only work on fundamental types");

    std::string name = "unknown fundamental";
    if constexpr (std::is_same_v<T, int>) {
        name = "int";
    } else if constexpr (std::is_same_v<T, long>) {
        name = "long";
    } else if constexpr (std::is_same_v<T, char>) {
        name = "char";
    } else if constexpr (std::is_same_v<T, short>) {
        name = "short";
    } else if constexpr (std::is_same_v<T, long long>) {
        name = "long long";
    } else if constexpr (std::is_same_v<T, long int>) {
        name = "long int";
    }

    if constexpr (!std::is_signed_v<T>) {
        name = "unsigned " + name;
    }

    return name;
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
            internal::get_node_category<T>(),
            "undefined",
            std::is_member_pointer_v<T>,
            std::is_pointer_v<T>,
            std::is_reference_v<T>,
            std::is_const_v<std::remove_reference_t<T>>,
            std::is_volatile_v<T>,
            std::is_array_v<T>,

            std::is_integral_v<T>,
            std::is_floating_point_v<T>,
            std::is_signed_v<T>,
            std::is_same_v<T, std::string>,
            util::is_container_v<T>,

            false,
        };
        if (!type) {
            type = &node;
            node.raw_type =
                info_node<util::completely_strip_type_t<T>>::resolve();
            if constexpr (std::is_fundamental_v<T>) {
                node.name = get_fundamental_type_name<T>();
            }

            using stripped_type = util::remove_cvref_t<T>;
            if constexpr (std::is_function_v<stripped_type> ||
                          std::is_member_function_pointer_v<stripped_type>) {
                node.is_member_pointer =
                    util::function_traits<stripped_type>::is_const;
            }
        }
        return type;
    }

    bool operator==(const info_node& node) const { return true; }

    bool operator!=(const info_node& node) const { return false; }
};

struct registry final {
    inline static std::vector<type_node*> nodes;
};

}  // namespace internal

}  // namespace drefl

}  // namespace mirrow