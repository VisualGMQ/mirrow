#pragma once

#include "mirrow/drefl/info_node.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/variable_traits.hpp"
#include "mirrow/util/type_list.hpp"
#include "mirrow/drefl/any.hpp"

namespace mirrow {

namespace drefl {

template <auto F>
struct func_traits {
    static any invoke(any* args) {
        using traits = util::function_pointer_traits<F>;
        return any{internal::invoke<F>(args, std::make_index_sequence<traits::args_with_class::size>())};
    }
};

template <auto F>
struct var_traits {
    using traits = util::variable_pointer_traits<F>;

    static any invoke(any* args) {
        if constexpr (traits::is_member) {
            return any{std::invoke(F, args->cast<typename traits::clazz*>())};
        } else {
            return any{*F};
        }
    }

    static unsigned long long cast_to_uint(void* instance) {
        return *(typename traits::type*)(instance);
    }

    static long long cast_to_int(void* instance) {
        return *(typename traits::type*)(instance);
    }

    static double cast_to_floating_point(void* instance) {
        return *(typename traits::type*)(instance);
    }
};

template <typename T>
struct factory final {
    static_assert(std::is_class_v<T>, "currently factory only support regist class");

    factory(const std::string& name) {
        internal::type_node* type = resolve();
        type->name = name;
        if (!internal::registry::nodes.empty()) {
            internal::registry::nodes.back()->next = type;
        }
        internal::registry::nodes.push_back(type);
    }

    template <typename... Args>
    factory& ctor() {
        internal::type_node* const type = resolve();
        static internal::ctor_node node = {
            type,
        };

        type->ctors.push_back(&node);
        internal::info_node<T>::template ctor<util::type_list<Args...>> = &node;

        return *this;
    }

    template <auto Func>
    factory& func(const std::string& name) {
        internal::type_node* const type = resolve();
        using traits = util::function_pointer_traits<Func>;

        static internal::function_node node = {
            type,
            name,
            util::function_pointer_traits<Func>::is_member,
            util::function_pointer_traits<Func>::is_const,
            &func_traits<Func>::invoke,
        };

        type->funcs.push_back(&node);
        internal::info_node<T>::template func<Func> = &node;

        return *this;
    }

    template <auto Func>
    factory& var(const std::string& name) {
        internal::type_node* const type = resolve();
        using traits = util::variable_pointer_traits<Func>;

        static internal::variable_node node = {
            type,
            name,
            traits::is_member,
            std::is_const_v<typename traits::type>,
            std::is_reference_v<typename traits::type>,
            std::is_pointer_v<typename traits::type>,
            std::is_integral_v<typename traits::type>,
            std::is_floating_point_v<typename traits::type>,
            std::is_signed_v<typename traits::type>,
            util::is_container_v<typename traits::type>,
            &var_traits<Func>::invoke,
        };

        type->vars.push_back(&node);
        internal::info_node<T>::template var<Func> = &node;

        return *this;
    }

private:
    internal::type_node* resolve() {
        return internal::info_node<T>::resolve();
    }
};

}

}