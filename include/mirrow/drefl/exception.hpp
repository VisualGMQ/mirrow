#pragma once

#include <exception>
#include <stdexcept>

namespace mirrow::drefl {

class bad_any_access: public std::logic_error {
public:
    using std::logic_error::logic_error;
};

class any_no_copyable: public std::logic_error {
public:
    using std::logic_error::logic_error;
};

}