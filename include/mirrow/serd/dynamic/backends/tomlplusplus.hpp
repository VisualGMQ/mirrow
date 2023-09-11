#pragma once

#ifndef TOML_EXCEPTIONS
#define TOML_EXCEPTIONS 0
#endif
#include "toml++/toml.hpp"

#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/drefl.hpp"
#include "mirrow/assert.hpp"

namespace mirrow {

namespace sred {

namespace drefl {

toml::table serialize_class(mirrow::drefl::any& data);

inline toml::array serialize_array(mirrow::drefl::any& data) {
    auto info = data.type_info();
    MIRROW_ASSERT(
        info.is_array() || (info.is_class() && info.as_class().is_container()),
        "data must a container or array in serialize_array()");

    toml::array arr;

    data.travel_elements([&](mirrow::drefl::any& elem, mirrow::drefl::type_info type) {
        if (type.is_array() || (type.is_class() && type.as_class().is_container())) {
            arr.push_back(serialize_array(elem));
        } else if (type.is_fundamental()) {
            auto fund = type.as_fundamental();
            if (fund.is_floating_pointer()) {
                arr.push_back(elem.try_cast_floating_point().value());
            } else if (fund.is_integral()) {
                arr.push_back(toml::int64_t(fund.is_signed() ? elem.try_cast_integral().value() : elem.try_cast_uintegral().value()));
            } else {
                MIRROW_LOG("unsupport fundamental type");
            }
        } else if (type.is_class()) {
            if (type.as_class().is_string()) {
                arr.push_back(elem.cast<std::string>());
            } else {
                arr.push_back(serialize_class(elem));
            }
        }
    });

    return arr;
}

inline toml::table serialize_class(mirrow::drefl::any& data) {
    mirrow::drefl::type_info info{data.type_info()};
    auto raw_info = info.raw_type();

    MIRROW_ASSERT(
        info.is_class() || (info.is_pointer() && raw_info.is_class()),
        "try to serialize a non-class type in serialize_calss()");

    toml::table tbl;

    for (auto& var : raw_info.as_class().vars()) {
        auto result = mirrow::drefl::invoke_by_any(var, &data);
        auto result_type = result.type_info();
        if (result_type.is_pointer()) {
            MIRROW_LOG("currently we can't serialize pointer type");
        } else if (result_type.is_fundamental()) {
            auto fund = result_type.as_fundamental();
            if (fund.is_floating_pointer()) {
                tbl.emplace(var.name(), result.try_cast_floating_point().value());
            } else if (fund.is_integral()) {
                if (fund.is_signed()) {
                    tbl.emplace(var.name(), result.try_cast_integral().value());
                } else {
                    // TODO: check casted any can convert to int64 losslessly
                    tbl.emplace(var.name(), (toml::int64_t)result.try_cast_uintegral().value());
                }
            }
        } else if (result_type.is_class() && result_type.as_class().is_string()) {
                tbl.emplace(var.name(), result.cast<std::string>());
        } else if (result_type.is_array() || (result_type.is_class() && result_type.as_class().is_container())) {
            tbl.emplace(var.name(), serialize_array(result));
        } else if (result_type.is_class()) {
            tbl.emplace(var.name(), serialize_class(result));
        }
    }

    return tbl;
}

}

}

}