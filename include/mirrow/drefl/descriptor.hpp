#pragma once

#include "mirrow/drefl/info_node.hpp"
#include "mirrow/drefl/any.hpp"

namespace mirrow {

namespace drefl {

class function_descriptor final {
public:
    using type = internal::function_node;

    explicit function_descriptor(const type& n)
        : node_(&n) {}

    std::string_view name() const { return node_->name; }
    auto parent() const { return node_->parent; }

    template <typename... Args>
    any invoke(Args&&... args) const {
        std::array<any, sizeof...(Args)> params = { any{std::forward<Args>(args)} ... };
        return node_->invoke(params.data());
    }

private:
    const type* node_;
};

class variable_descriptor final {
public:
    using type = internal::variable_node;

    explicit variable_descriptor(const type& n)
        : node_(&n) {}

    std::string_view name() const { return node_->name; }
    auto parent() const { return node_->parent; }

    template <typename... Args>
    any invoke(Args&&... args) const {
        std::array<any, sizeof...(Args)> params = { any{std::forward<Args>(args)} ... };
        return node_->invoke(params.data());
    }

private:
    const type* node_;
};

template <typename T, typename Descriptor>
class field_container final {
public:
    class iterator final {
    public:
        iterator(const std::vector<T*>& container, size_t idx): container_(container), idx_(idx) {}

        bool operator==(const iterator& o) const {
            return &o.container_ == &container_ && o.idx_ == idx_;
        }

        bool operator!=(const iterator& o) const {
            return !(*this == o);
        }

        iterator& operator+=(size_t step) {
            return idx_ += step, *this;
        }

        iterator& operator-=(size_t step) {
            return idx_ -= step, *this;
        }

        iterator operator+(size_t step) {
            iterator tmp = *this;
            return tmp += step;
        }

        iterator operator-(size_t step) {
            iterator tmp = *this;
            return tmp -= step;
        }

        iterator& operator++() {
            return *this += 1, *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            return ++(*this), tmp;
        }

        iterator& operator--() {
            return *this -= 1, *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            return --(*this), tmp;
        }

        function_descriptor operator*() {
            return function_descriptor{*container_[idx_]};
        }

    private:
        const std::vector<T*>& container_;
        size_t idx_;
    };

    using value_type = T;
    using reference = value_type&;
    using const_reference = const reference;
    using pointer = value_type*;
    using const_pointer = const pointer;
    using size_type = typename std::vector<T*>::size_type;
    using difference_type = typename std::vector<T*>::difference_type;
    using allocator_type = typename std::vector<T*>::allocator_type;
    using const_iterator = const iterator;


    field_container(const std::vector<T*>& nodes): nodes_(nodes) {}

    size_t size() const { return nodes_.size(); }

    Descriptor operator[](size_t size) {
        return Descriptor{*nodes_[size]};
    }

    const_iterator begin() const {
        return iterator{nodes_, 0};
    }

    const_iterator end() const {
        return iterator{nodes_, nodes_.size()};
    }

    iterator begin() {
        return const_cast<iterator&&>(std::as_const(*this).begin());
    }

    iterator end() {
        return const_cast<iterator&&>(std::as_const(*this).end());
    }

private:
    const std::vector<T*>& nodes_;
};

using function_container = field_container<typename function_descriptor::type, function_descriptor>;
using variable_container = field_container<typename variable_descriptor::type, variable_descriptor>;


class fundamental_type final {
public:
    explicit fundamental_type(const internal::type_node* type)
        : type_(type) {}

    bool is_integral() const { return type_->is_integral; }
    bool is_floating_pointer() const { return type_->is_floating_pointer; }
    bool is_signed() const { return type_->is_signed; }

private:
    const internal::type_node* type_;
};

class function_type final {
public:
    explicit function_type(const internal::type_node* type)
        : type_(type) {}

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
    explicit enum_type(const internal::type_node* type)
        : type_(type) {}

private:
    const internal::type_node* type_;
};

class class_type final {
public:
    explicit class_type(const internal::type_node* type)
        : type_(type) {}

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

    auto category() const { return type_->category; }

    bool is_fundamental() const { return type_->category == type_category::Fundamental; }
    bool is_class() const { return type_->category == type_category::Class; }
    bool is_function() const { return type_->category == type_category::Function; }
    bool is_member_function() const { return type_->category == type_category::MemberFunction; }
    bool is_member_object() const { return type_->category == type_category::MemberObject; }
    bool is_enum() const { return type_->category == type_category::Enum; }
    
    bool is_array() const { return type_->is_array; }
    bool is_reference() const { return type_->is_reference; }
    bool is_const() const { return type_->is_const; }
    bool is_volatile() const { return type_->is_volatile; }
    bool is_compound_type() const { return type_->raw_type != type_; }

    const std::string& name() const { return type_->name; }

    type_info raw_type() const { return type_info{type_->raw_type}; }

    fundamental_type as_fundamental() const { return fundamental_type{type_}; }
    function_type as_function() const { return function_type{type_}; }
    member_function_type as_member_function() const { return member_function_type{type_}; }
    member_object_type as_member_object() const { return member_object_type{type_}; }
    enum_type as_enum() const { return enum_type{type_}; }
    class_type as_class() const { return class_type{type_}; }

    const internal::type_node* type_node() const { return type_; }

    bool operator==(const type_info& o) const {
        return o.type_ == type_;
    }

    bool operator!=(const type_info& o) const {
        return !(o == *this);
    }

private:
    const internal::type_node* type_;
};


}

}