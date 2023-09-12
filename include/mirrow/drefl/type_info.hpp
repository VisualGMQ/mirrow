#pragma once

#include "mirrow/drefl/descriptor.hpp"
#include "mirrow/drefl/info_node.hpp"

namespace mirrow {

namespace drefl {

class fundamental_type final {
public:
    explicit fundamental_type(const internal::type_node* type) : type_(type) {}

    bool is_integral() const { return type_->is_integral; }

    bool is_floating_pointer() const { return type_->is_floating_pointer; }

    bool is_signed() const { return type_->is_signed; }

private:
    const internal::type_node* type_;
};

class function_type final {
public:
    explicit function_type(const internal::type_node* type) : type_(type) {}

private:
    const internal::type_node* type_;
};

class member_function_type final {
public:
    explicit member_function_type(const internal::type_node* type)
        : type_(type) {}

    bool is_const_member() const { return type_->is_const_member; }

private:
    const internal::type_node* type_;
};

class member_object_type final {
public:
    explicit member_object_type(const internal::type_node* type)
        : type_(type) {}

private:
    const internal::type_node* type_;
};

class enum_type final {
public:
    explicit enum_type(const internal::type_node* type) : type_(type) {}

private:
    const internal::type_node* type_;
};

class class_type final {
public:
    explicit class_type(const internal::type_node* type) : type_(type) {}

    bool is_string() const { return type_->is_string; }

    bool is_container() const { return type_->is_container; }

    auto funcs() const { return function_container{type_->funcs}; }

    auto vars() const { return variable_container{type_->vars}; }

private:
    const internal::type_node* type_;
};

class type_info final {
public:
    explicit type_info(const internal::type_node* type) : type_(type) {}

    operator bool() const { return has_value(); }

    bool has_value() const { return type_ != nullptr; }

    auto category() const { return type_->category; }

    bool is_fundamental() const {
        return type_->category == type_category::Fundamental;
    }

    bool is_class() const { return type_->category == type_category::Class; }

    bool is_function() const {
        return type_->category == type_category::Function;
    }

    bool is_member_function() const {
        return type_->category == type_category::MemberFunction;
    }

    bool is_member_object() const {
        return type_->category == type_category::MemberObject;
    }

    bool is_enum() const { return type_->category == type_category::Enum; }

    bool is_array() const { return type_->is_array; }

    bool is_reference() const { return type_->is_reference; }

    bool is_const() const { return type_->is_const; }

    bool is_volatile() const { return type_->is_volatile; }

    bool is_compound_type() const { return type_->raw_type != type_; }

    bool is_pointer() const { return type_->is_pointer; }

    const std::string& name() const { return type_->name; }

    type_info raw_type() const { return type_info{type_->raw_type}; }

    fundamental_type as_fundamental() const { return fundamental_type{type_}; }

    function_type as_function() const { return function_type{type_}; }

    member_function_type as_member_function() const {
        return member_function_type{type_};
    }

    member_object_type as_member_object() const {
        return member_object_type{type_};
    }

    enum_type as_enum() const { return enum_type{type_}; }

    class_type as_class() const { return class_type{type_}; }

    const internal::type_node* type_node() const { return type_; }

    bool operator==(const type_info& o) const { return o.type_ == type_; }

    bool operator!=(const type_info& o) const { return !(o == *this); }

private:
    const internal::type_node* type_;
};

}  // namespace drefl

}  // namespace mirrow

// hash support for type_info
namespace std {
    template <>
    struct hash<mirrow::drefl::type_info> {
        std::size_t operator()(const mirrow::drefl::type_info& type) const {
            return std::hash<const mirrow::drefl::internal::type_node*>{}(type.type_node());
        }
    };
}