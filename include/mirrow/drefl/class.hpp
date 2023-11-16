#pragma once

#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/qualifier.hpp"
#include "mirrow/util/misc.hpp"
#include <memory>
#include <type_traits>
#include <vector>

namespace mirrow::drefl {

#define SET_QUALIFIER_BIT(qualif, newBit)                       \
    qualif = static_cast<qualifier>(static_cast<long>(qualif) | \
                                    static_cast<long>(newBit))

template <typename T>
qualifier get_qualifier() {
    qualifier qualif = qualifier::None;

    if constexpr (std::is_lvalue_reference_v<T>) {
        SET_QUALIFIER_BIT(qualif, qualifier::Ref);
        if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
            SET_QUALIFIER_BIT(qualif, qualifier::Const);
        }
    }
    if constexpr (std::is_const_v<T>) {
        SET_QUALIFIER_BIT(qualif, qualifier::Const);
    }

    return qualif;
}

#undef SET_QUALIFIER_BIT

class property;
class any;

class clazz final : public type {
public:
    template <typename T>
    friend class class_factory;

    explicit clazz(const std::string& name): type(value_kind::Class, name) {}
    clazz(): type(value_kind::Class) {}

    auto& properties() const noexcept { return properties_; }

    void set_value(any& from, any& to);
    void steal_value(any& from, any& to);

private:
    std::vector<std::shared_ptr<property>> properties_;
};

}
