#include "mirrow/drefl/string.hpp"
#include "mirrow/assert.hpp"
#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/drefl/make_any.hpp"


namespace mirrow::drefl {

string::string(enum string_kind skind, const std::string& name)
    : type(value_kind::String, name,
           [=]() {
               if (skind == string_kind::String) {
                   return any_make_copy<std::string>("");
               } else {
                   return any_make_copy<std::string_view>("");
               }
           }),
      kind_(skind) {}

void string::set_value(any& a, const std::string& value) const {
    if (!SET_VALUE_CHECK(a, value_kind::String)) return;

    switch (a.type_info()->as_string()->string_kind()) {
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

    switch (a.type_info()->as_string()->string_kind()) {
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

std::string string::get_str(const any& value) const {
    if (value.type_info()->as_string()->is_string_view()) {
        return std::string(*(std::string_view*)(value.payload()));
    } else {
        return *(std::string*)(value.payload());
    }
}

std::string_view string::get_str_view(const any& value) const {
    if (value.type_info()->as_string()->is_string_view()) {
        return *(std::string_view*)(value.payload());
    } else {
        return *(std::string*)(value.payload());
    }
}

}  // namespace mirrow::drefl