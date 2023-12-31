#pragma once

#include "mirrow/drefl/type.hpp"

namespace mirrow::drefl {

class any;

class boolean final: public type {
public:
    boolean();

    void set_value(any&, bool) const;
    bool get_value(const any&) const;
};

}