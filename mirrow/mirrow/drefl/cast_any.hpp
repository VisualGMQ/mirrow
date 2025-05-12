#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/factory.hpp"

namespace mirrow::drefl {

template <typename T>
T* try_cast(any& a) {
    if (typeinfo<T>() == a.type_) {
        if (a.access_ == any::access_type::Ref || a.access_ == any::access_type::Copy) {
            return static_cast<T*>(a.payload_);
        }
    }
    throw bad_any_access{"can't cast mutable type from const reference"};
    return nullptr;
}

template <typename T>
const T* try_cast_const(const any& a) {
    if (typeinfo<T>() == a.type_) {
        return static_cast<T*>(a.payload_);
    }
    return nullptr;
}


}