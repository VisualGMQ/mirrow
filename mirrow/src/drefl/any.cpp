#include "mirrow/drefl/any.hpp"

namespace mirrow::drefl {

any::any(any&& o)
    : access_(o.access_), payload_(o.payload_), operations_(o.operations_), type_(o.type_) {
    o.access_ = access_type::Null;
    o.payload_ = nullptr;
}

any::any(const any& o) {
    switch (o.access_) {
        case access_type::Null:
            access_ = access_type::Null;
            payload_ = nullptr;
            operations_ = &type_operations::null;
            break;
        case access_type::ConstRef:
        case access_type::Ref:
            payload_ = o.payload_;
            access_ = o.access_;
            operations_ = o.operations_;
            break;
        case access_type::Copy:
            payload_ = o.operations_->copy_construct(o.payload_);
            access_ = o.access_;
            operations_ = o.operations_;
            break;
    }
    type_ = o.type_;
}

any& any::operator=(const any& o) {
    if (access_ == access_type::Copy) {
        operations_->destroy(payload_);
        payload_ = nullptr;
    }

    type_ = o.type_;

    switch (o.access_) {
        case access_type::Null:
            access_ = access_type::Null;
            payload_ = nullptr;
            operations_ = &type_operations::null;
            break;
        case access_type::ConstRef:
        case access_type::Ref:
            access_ = o.access_;
            payload_ = o.payload_;
            operations_ = o.operations_;
            break;
        case access_type::Copy:
            access_ = o.access_;
            payload_ = o.operations_->copy_construct(o.payload_);
            operations_ = o.operations_;
            break;
    }

    return *this;
}

any& any::operator=(any&& o) {
    if (&o != this) {
        payload_ = o.payload_;
        access_ = o.access_;
        operations_ = o.operations_;
        type_ = o.type_;

        o.payload_ = nullptr;
        o.access_ = access_type::Null;
        o.operations_ = &type_operations::null;
        o.type_ = nullptr;
    }
    return *this;
}

any::~any() {
    if (access_ == access_type::Copy && operations_) {
        operations_->destroy(payload_);
    }
}

}  // namespace mirrow::drefl