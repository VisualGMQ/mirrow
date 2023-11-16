#include "mirrow/drefl/numeric.hpp"
#include "mirrow/assert.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/drefl/value_kind.hpp"


namespace mirrow::drefl {

template <typename T>
void do_set_value(any& a, T value) {
    switch (a.type_info()->as_numeric()->numeric_kind()) {
        case numeric::Unknown:
            MIRROW_LOG("any has unknown numeric type");
            break;
        case numeric::Char:
            *try_cast<char>(a) = value;
            break;
        case numeric::Int:
            *try_cast<int>(a) = value;
            break;
        case numeric::Short:
            *try_cast<short>(a) = value;
            break;
        case numeric::Long:
            *try_cast<long>(a) = value;
            break;
        case numeric::Uint8:
            *try_cast<uint8_t>(a) = value;
            break;
        case numeric::Uint16:
            *try_cast<uint16_t>(a) = value;
            break;
        case numeric::Uint32:
            *try_cast<uint32_t>(a) = value;
            break;
        case numeric::Uint64:
            *try_cast<uint64_t>(a) = value;
            break;
        case numeric::Float:
            *try_cast<float>(a) = value;
            break;
        case numeric::Double:
            *try_cast<double>(a) = value;
            break;
            break;
    }
}

void numeric::set_value(any& a, long value) const {
    if (!SET_VALUE_CHECK(a, value_kind::Numeric)) {
        MIRROW_LOG("can't set boolean value to any");
        return;
    }

    do_set_value(a, value);
}

void numeric::set_value(any& a, uint64_t value) const {
    if (!SET_VALUE_CHECK(a, value_kind::Numeric)) {
        MIRROW_LOG("can't set boolean value to any");
        return;
    }

    do_set_value(a, value);
}

void numeric::set_value(any& a, double value) const {
    if (!SET_VALUE_CHECK(a, value_kind::Numeric)) {
        MIRROW_LOG("can't set boolean value to any");
        return;
    }

    do_set_value(a, value);
}

double numeric::get_value(const any& a) const {
    if (a.type_info()->kind() != value_kind::Numeric) return 0;

    switch (a.type_info()->as_numeric()->numeric_kind()) {
        case Unknown:
            return 0;
        case Char:
            return *try_cast_const<char>(a);
        case Int:
            return *try_cast_const<int>(a);
        case Short:
            return *try_cast_const<short>(a);
        case Long:
            return *try_cast_const<long>(a);
        case Uint8:
            return *try_cast_const<uint8_t>(a);
        case Uint16:
            return *try_cast_const<uint16_t>(a);
        case Uint32:
            return *try_cast_const<uint32_t>(a);
        case Uint64:
            return *try_cast_const<uint64_t>(a);
        case Float:
            return *try_cast_const<float>(a);
        case Double:
            return *try_cast_const<double>(a);
    }

    return 0;
}

}  // namespace mirrow::drefl