#pragma once

#include "mirrow/drefl/info_node.hpp"
#include "mirrow/util/function_traits.hpp"
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

template <typename T>
struct factory final {
    static_assert(std::is_class_v<T>, "currently factory only support regist class");

    factory(const std::string& name) {
        internal::type_node* type = resolve();
        type->name = name;
        if (internal::registry::root) {
            internal::registry::root = type;
        } else {
            type->next = internal::registry::root;
            internal::registry::root = type;
        }
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

private:
    internal::type_node* resolve() {
        return internal::info_node<T>::resolve();
    }
};

}

}