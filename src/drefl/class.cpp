#include "mirrow/drefl/class.hpp"
#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/any.hpp"

namespace mirrow::drefl {

void clazz::set_value(any& from, any& to) {
    if (!COPY_VALUE_CHECK(from, value_kind::Class) ||
        !SET_VALUE_CHECK(to, value_kind::Class))
        return;

    if (from.type_info() != to.type_info()) return;

    to.operations_->copy_assignment(to.payload(), from.payload());
}

void clazz::steal_value(any& from, any& to) {
    if (!COPY_VALUE_CHECK(from, value_kind::Class) ||
        !SET_VALUE_CHECK(to, value_kind::Class))
        return;

    if (from.type_info() != to.type_info()) return;

    to.operations_->steal_assignment(to.payload(), from.payload());
}

any clazz::default_construct() const {
    if (default_construct_) {
        return default_construct_();
    } else {
        return {};
    }
}

}  // namespace mirrow::drefl