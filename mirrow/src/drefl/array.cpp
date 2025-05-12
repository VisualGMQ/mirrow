#include "mirrow/drefl/array.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/operation_traits.hpp"

namespace mirrow::drefl {

any array::get(size_t idx, any& array) const noexcept {
    enum any::access_type access = any::access_type::Ref;
    if (array.is_constref()) {
        access = any::access_type::ConstRef;
    }
    return {access, operations_->get(idx, array.payload(), false),
            elem_operations_, elem_type_};
}

any array::get_const(size_t idx, const any& array) const noexcept {
    enum any::access_type access = any::access_type::Ref;
    return {any::access_type::ConstRef,
            operations_->get(idx, (void*)array.payload(), true),
            elem_operations_, elem_type_};
}

bool array::push_back(const any& elem, any& array) const noexcept {
    if (elem.type_info() != elem_type_) {
        return false;
    }

    return operations_->push_back(elem.payload(), array.payload());
}

bool array::pop_back(any& array) const noexcept {
    if (array.is_constref()) {
        return false;
    } else {
        return operations_->pop_back(array.payload());
    }
}

any array::back(any& array) const noexcept {
    void* elem = operations_->back(array.payload(), false);

    if (!elem) {
        return {any::access_type::Null, nullptr, nullptr, array.type_info()};
    } else {
        auto access = array.access_type() == any::access_type::ConstRef
                          ? any::access_type::ConstRef
                          : any::access_type::Ref;
        return {access, elem, elem_operations_, elem_type_};
    }
}

any array::back_const(const any& array) const noexcept {
    void* elem = operations_->back((void*)array.payload(), true);

    if (!elem) {
        return {any::access_type::Null, nullptr, nullptr, array.type_info()};
    } else {
        return {any::access_type::ConstRef, elem, elem_operations_, elem_type_};
    }
}

bool array::resize(size_t size, any& array) const noexcept {
    if (array.is_constref()) {
        return false;
    }
    return operations_->resize(size, array.payload());
}

size_t array::size(const any& array) const noexcept {
    return operations_->size(array.payload());
}

size_t array::capacity(const any& array) const noexcept {
    return operations_->capacity(array.payload());
}

bool array::insert(size_t idx, const any& elem, any& array) const noexcept {
    if (array.is_constref() || elem.type_info() != elem_type_) {
        return false;
    }

    return operations_->insert(idx, elem.payload(), array.payload());
}

}  // namespace mirrow::drefl