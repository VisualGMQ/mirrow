#pragma once

#include "mirrow/drefl/factory.hpp"

namespace mirrow::drefl {

class class_visitor {
public:
    virtual ~class_visitor() = default;

    virtual void operator()(numeric_property&) = 0;
    virtual void operator()(enum_property&) = 0;
    virtual void operator()(clazz_property&) = 0;
    virtual void operator()(string_property&) = 0;
    virtual void operator()(boolean_property&) = 0;
    virtual void operator()(pointer_property&) = 0;
    virtual void operator()(array_property&) = 0;
};

}  // namespace mirrow::drefl