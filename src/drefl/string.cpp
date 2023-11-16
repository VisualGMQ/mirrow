#include "mirrow/drefl/string.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/assert.hpp"

namespace mirrow::drefl {

void string::set_value(any& a, const std::string& value) const {
    if (!SET_VALUE_CHECK(a, value_kind::String)) return;

    switch(a.type_info()->as_string()->string_kind()) {
        case string_kind::Unknown:
            MIRROW_LOG("unknown string type");
            break;
        case string_kind::String:
            *try_cast<std::string>(a) = value;
            break;
        case string_kind::StringView:
            *try_cast<std::string_view>(a) = value;
            break;
    }
}

void string::set_value(any& a, std::string_view& value) const {
    if (!SET_VALUE_CHECK(a, value_kind::String)) return;

    switch(a.type_info()->as_string()->string_kind()) {
        case string_kind::Unknown:
            MIRROW_LOG("unknown string type");
            break;
        case string_kind::String:
            *try_cast<std::string>(a) = value;
            break;
        case string_kind::StringView:
            *try_cast<std::string_view>(a) = value;
            break;
    }
}

std::string_view string::get_str_view(const any& value) const {
    if (value.type_info()->as_string()->is_string_view()) {
        return *(std::string_view*)(value.payload());
    } else {
        return *(std::string*)(value.payload());
    }
}

}