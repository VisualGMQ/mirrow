#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/class_visitor.hpp"

namespace mirrow::drefl {

void numeric_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

void enum_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

void clazz_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

void string_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

void boolean_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

void pointer_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

void array_property::visit(class_visitor* visitor) {
    (*visitor)(*this);
}

}  // namespace mirrow::drefl