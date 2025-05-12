#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/assert.hpp"

namespace mirrow::drefl {

template <typename T>
any any_make_constref(const T& value) noexcept {
    using type = util::remove_cvref_t<T>;
    return {any::access_type::ConstRef, (void*)&value,
            &type_operation_traits<type>::get_operations(),
            typeinfo<util::remove_cvref_t<T>>()};
}

template <typename T>
any any_make_ref(T& value) noexcept {
    using type = util::remove_cvref_t<T>;
    return {any::access_type::Ref, (void*)&value,
            &type_operation_traits<type>::get_operations(),
            typeinfo<util::remove_cvref_t<T>>()};
}

template <typename T>
any any_make_copy(T&& value) noexcept(
    std::is_rvalue_reference_v<T&&>
        ? std::is_nothrow_move_constructible_v<util::remove_cvref_t<T>>
        : std::is_nothrow_copy_constructible_v<util::remove_cvref_t<T>>) {
    using type = util::remove_cvref_t<T>;

    void* elem = nullptr;
    try {
        if constexpr (std::is_enum_v<T>) {
            elem = new long{static_cast<long>(value)};
        } else {
            elem = new type{std::forward<T>(value)};
        }
    } catch (const std::bad_alloc&) {
        MIRROW_LOG("make copy failed! due to allocate memory failed!");
        elem = nullptr;
    } catch (...) {
        throw;
    }
    return {any::access_type::Copy, elem, &type_operation_traits<type>::get_operations(),
            typeinfo<util::remove_cvref_t<T>>()};
}

}  // namespace mirrow::drefl