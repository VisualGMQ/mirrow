#pragma once

#include "info_node.hpp"
#include "mirrow/util/function_traits.hpp"

namespace mirrow {

namespace drefl {

template <typename T>
struct factory final {
    factory(std::string_view name) {
        internal::type_node* const type = resolve();
        type->name = name;
        if (internal::registry::root) {
            internal::registry::root = &type;
        }
    }

    template <auto Func>
    factory& ctor() {
        internal::type_node* const type = resolve();
        using traits = function_pointer_traits<Func>;

        static internal::ctor_node node = {
            &resolve(),
            type->ctor,
        };

        type->ctor = &node;
        internal::info_node<T>::ctor<typename traits::args> = &node;

        return *this;
    }

private:
    internal::type_node* resolve() {
        return internal::info_node<T>::resolve();
    }
};

}

}