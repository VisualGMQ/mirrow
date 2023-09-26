#pragma once

#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/variable_traits.hpp"
#include "mirrow/util/misc.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace mirrow {

namespace drefl {

class basic_any;
class any;
class reference_any;

enum class type_category {
    Compound,        // a compound type(with qualifier/reference/pointer/array)
    Fundamental,     // fundamental type(without pointer)
    Class,           // class type
    Function,        // function type: Ret(Param...)
    MemberFunction,  // member function type: Ret(Class::*)(Param...)
    MemberObject,    // member variable type: Ret Class::*
    Enum,            // enumeration
};

enum class container_type {
    NotContainer,   // not a container
    FlatArray,      // flat array(like int[3])
    Array,          // std::array
    Vector,         // std::vector
    UnorderedMap,   // std::unordered_map
    Map,            // std::map
    UnorderedSet,   // std::unordered_set
    Set,            // std::set
    String,         // std::string
    Custom,         // user custom container
};

namespace internal {

struct type_node;

struct ctor_node final {
    type_node* parent = nullptr;

    std::vector<type_node*> params;

    any (*invoke)(basic_any*) = nullptr;
};

struct function_node final {
    type_node* parent = nullptr;
    type_node* type = nullptr;
    std::string name;
    bool is_const_member;
    any (*invoke)(basic_any*) = nullptr;
    reference_any (*invoke_by_ref)(basic_any*) = nullptr;
};

struct variable_node final {
    type_node* parent = nullptr;
    type_node* type = nullptr;
    std::string name;
    any (*invoke)(basic_any*) = nullptr;
    reference_any (*invoke_by_ref)(basic_any*) = nullptr;
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
    bool is_boolean;
    bool is_signed;
    bool is_default_constructable;

    enum container_type container_type;
    type_node* container_value_type;

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
    } else if constexpr (std::is_same_v<T, float>) {
        name = "float";
    } else if constexpr (std::is_same_v<T, double>) {
        name = "double";
    } else if constexpr (std::is_same_v<T, bool>) {
        name = "bool";
    }

    if constexpr (!std::is_signed_v<T>) {
        name = "unsigned " + name;
    }

    return name;
}

template <typename T>
container_type get_node_container_type() {
    if constexpr (util::is_std_array_v<T>) {
        return container_type::Array;
    } else if constexpr (util::is_vector_v<T>) {
        return container_type::Vector;
    } else if constexpr (util::is_unordered_map_v<T>) {
        return container_type::UnorderedMap;
    } else if constexpr (util::is_map_v<T>) {
        return container_type::Map;
    } else if constexpr (util::is_set_v<T>) {
        return container_type::Set;
    } else if constexpr (util::is_unordered_set_v<T>) {
        return container_type::UnorderedSet;
    } else if constexpr (util::is_string_v<T>) {
        return container_type::String;
    } else if constexpr (util::is_container_v<T>) {
        return container_type::Custom;
    } else {
        return container_type::NotContainer;
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
            std::is_same_v<T, bool>,
            std::is_signed_v<T>,
            std::is_default_constructible_v<T>,

            get_node_container_type<T>(),
            nullptr,
            false,
            nullptr,
        };
        if (!type) {
            type = &node;

            if constexpr (std::is_member_object_pointer_v<T>) {
                node.raw_type =
                    info_node<util::completely_strip_type_t<util::variable_type_t<T>>>::resolve();
            } else {
                node.raw_type =
                    info_node<util::completely_strip_type_t<T>>::resolve();
            }
            if constexpr (std::is_fundamental_v<T>) {
                node.name = get_fundamental_type_name<T>();
            }

            using stripped_type = util::remove_cvref_t<T>;
            if constexpr (std::is_function_v<stripped_type> ||
                          std::is_member_function_pointer_v<stripped_type>) {
                node.is_member_pointer =
                    util::function_traits<stripped_type>::is_const;
            }

            if constexpr (util::is_container_v<stripped_type>) {
                node.container_value_type = info_node<typename stripped_type::value_type>::resolve();
            }

            if constexpr (!std::is_array_v<T> && std::is_default_constructible_v<T>) {
                if (std::is_array_v<T>) {
                    MIRROW_LOG("I'm so sorry mirrow don't support default construct array in any");
                } else {
                    constexpr auto invoke = [](basic_any* args) {
                        return any{T{}};
                    };
                    static ctor_node default_ctor = {
                        &node,
                        {},
                        +invoke,
                    };

                    node.ctors.push_back(&default_ctor);
                }
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