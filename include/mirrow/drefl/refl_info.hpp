#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/info_node.hpp"

#include <array>
#include <utility>

namespace mirrow {

namespace drefl {

class refl_info final {
public:
    refl_info(internal::type_node* node) : type_(node) {}

    std::string_view name() const { return type_->name; }

    operator bool() const { return type_ != nullptr; }

    auto funcs() const { return function_container{type_->funcs}; }
    auto vars() const { return variable_container{type_->vars}; }

private:
    internal::type_node* type_;
};

}  // namespace drefl

}  // namespace mirrow