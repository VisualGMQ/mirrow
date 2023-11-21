#include "mirrow/drefl/numeric.hpp"
#include "mirrow/assert.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/drefl/make_any.hpp"


namespace mirrow::drefl {

any default_create(enum numeric::numeric_kind kind) {
    switch (kind) {
        case numeric::Unknown:
            return {};
        case numeric::Char:
            return any_make_copy<char>(0);
        case numeric::Int:
            return any_make_copy<int>(0);
        case numeric::Short:
            return any_make_copy<short>(0);
        case numeric::Long:
            return any_make_copy<long>(0);
        case numeric::Uint8:
            return any_make_copy<uint8_t>(0);
        case numeric::Uint16:
            return any_make_copy<uint16_t>(0);
        case numeric::Uint32:
            return any_make_copy<uint32_t>(0);
        case numeric::Uint64:
            return any_make_copy<uint64_t>(0);
        case numeric::Float:
            return any_make_copy<float>(0);
        case numeric::Double:
            return any_make_copy<double>(0);
    }

    return {};
}

numeric::numeric(value_kind value_kind, enum numeric_kind numeric_kind, const std::string& name)
        : type(value_kind, name), kind_(numeric_kind), default_construct_(default_create) {}

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