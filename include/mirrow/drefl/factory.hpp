#pragma once

#include "mirrow/assert.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/info_node.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/type_list.hpp"
#include "mirrow/util/variable_traits.hpp"
#include "mirrow/drefl/invoke_util.hpp"

namespace mirrow {

namespace drefl {

template <auto F>
struct func_traits {
    static any invoke(basic_any* args) {
        using traits = util::function_pointer_traits<F>;
        return any{invoke_by_any<F>(
            args, std::make_index_sequence<traits::args_with_class::size>())};
    }

    static reference_any invoke_by_ref(basic_any* args) {
        using traits = util::function_pointer_traits<F>;
        MIRROW_ASSERT(std::is_reference_v<typename traits::return_type>, "function's return type must be reference");
        if constexpr (std::is_reference_v<typename traits::return_type>) {
            return reference_any{invoke_by_any<F>(args, std::make_index_sequence<traits::args_with_class::size>())};
        } else {
            return reference_any{nullptr};
        }
    }
};

template <typename T, typename... Args>
struct ctor_traits {
    static any invoke(basic_any* args) {
        return do_invoke(args, std::make_index_sequence<sizeof...(Args)>());
    }

private:
    template <size_t... Indices>
    static any do_invoke(basic_any* args, std::index_sequence<Indices...>) {
        return any{T(((args + Indices)
                          ->cast<util::remove_cvref_t<util::list_element_t<
                              util::type_list<Args...>, Indices>>>())...)};
    }
};

template <auto F>
struct var_traits {
    using traits = util::variable_pointer_traits<F>;

    static any invoke(basic_any* args) {
        auto info = args->type();
        using clazz_type = typename traits::clazz;
        if constexpr (traits::is_member) {
            if (info.is_pointer()) {
                return any{std::invoke(F, args->cast<clazz_type*>())};
            } else {
                return any{std::invoke(F, args->cast<clazz_type&>())};
            }
        } else {
            return any{*F};
        }
    }

    static reference_any invoke_by_ref(basic_any* args) {
        auto info = args->type();
        using clazz_type = typename traits::clazz;
        if constexpr (traits::is_member) {
            if (info.is_pointer()) {
                return reference_any{std::invoke(F, args->cast<clazz_type*>())};
            } else {
                return reference_any{std::invoke(F, args->cast<clazz_type&>())};
            }
        } else {
            return reference_any{*F};
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
    factory(const std::string& name) {
        static_assert(!util::is_complex_type_v<T>,
                      "you can't reflect complex type(with "
                      "const/volatile/reference/pointer/array)\n(hint: you can "
                      "use mirrow::util::completely_strip_type_t to convert "
                      "complex type to pure type)");

        if (internal::info_node<T>::type) {
            MIRROW_LOG(name + " class has registed");
        }
        internal::type_node* type = resolve();
        type->name = name;
        internal::registry::nodes.push_back(type);
    }

    template <typename... Args>
    factory& ctor() {
        internal::type_node* const type = resolve();
        static internal::ctor_node node = {
            type,
            {},
            &ctor_traits<T, Args...>::invoke,
        };

        if constexpr (sizeof...(Args) > 0) {
            node.params = { (internal::info_node<Args>::resolve(), ...) };
        }

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
            internal::info_node<decltype(Func)>::resolve(),
            name,
            util::function_pointer_traits<Func>::is_const,
            &func_traits<Func>::invoke,
            &func_traits<Func>::invoke_by_ref,
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
            internal::info_node<decltype(Func)>::resolve(),
            name,
            &var_traits<Func>::invoke,
            &var_traits<Func>::invoke_by_ref,
        };

        type->vars.push_back(&node);
        internal::info_node<T>::template var<Func> = &node;

        return *this;
    }

private:
    internal::type_node* resolve() { return internal::info_node<T>::resolve(); }
};

}  // namespace drefl

}  // namespace mirrow