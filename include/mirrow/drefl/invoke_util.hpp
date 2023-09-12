#pragma once

#include <functional>

namespace mirrow {

namespace drefl {

template <auto Func, size_t... Indices>
decltype(auto) invoke_by_any(basic_any* args, std::index_sequence<Indices...>) {
    using fn_traits = util::function_pointer_traits<Func>;

    static_assert(fn_traits::args_with_class::size == sizeof...(Indices),
                  "the number of function call arguments is inconsistent");

    return std::invoke(
        Func, ((args + Indices)
                   ->cast<util::remove_cvref_t<util::list_element_t<
                       typename fn_traits::args_with_class, Indices>>>())...);
}


}

}