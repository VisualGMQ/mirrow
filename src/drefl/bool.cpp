#include "mirrow/drefl/bool.hpp"
#include "mirrow/assert.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/drefl/exception.hpp"

namespace mirrow::drefl {

void boolean::set_value(any& dst, bool value) const {
    if (!SET_VALUE_CHECK(dst, value_kind::Boolean)) {
        MIRROW_LOG("can't set boolean value to any");
        return;
    }

    *try_cast<bool>(dst) = value;
}

bool boolean::get_value(const any& value) const {
    return *try_cast_const<bool>(value);
}

}