#pragma once

#include "mirrow/drefl/property.hpp"
#include "mirrow/drefl/class.hpp"
#include "mirrow/drefl/property_factory.hpp"

namespace mirrow::drefl {


template <typename T>
class class_factory final {
public:
    static auto& instance() noexcept {
        static class_factory inst;
        return inst;
    }

    auto& regist(const std::string& name) {
        info_.name_ = name;

        return *this;
    }

    template <typename U>
    auto& property(const std::string& name, U accessor) {
        using traits = util::variable_traits<U>;
        using type = typename traits::type;

        info_.properties_.emplace_back(
            property_factory{}.create(name, accessor));

        return *this;
    }

    auto& info() const noexcept { return info_; }

private:
    clazz info_;
};


}