#pragma once

#include "mirrow/drefl/info_node.hpp"

#include <utility>
#include <array>

namespace mirrow {

namespace drefl {

class function_descriptor final {
public:
    explicit function_descriptor(const internal::function_node& n)
        : node_(&n) {}

    bool is_const() const { return node_->is_const; }
    bool is_member() const { return node_->is_member; }
    std::string_view name() const { return node_->name; }
    auto parent() const { return node_->parent; }

    template <typename... Args>
    any invoke(Args&&... args) const {
        std::array<any, sizeof...(Args)> params = { any{std::forward<Args>(args)} ... };
        return node_->invoke(params.data());
    }

// private:
    const internal::function_node* node_;
};

class func_container final {
public:
    class iterator final {
    public:
        iterator(const std::vector<internal::function_node*>& container, size_t idx): container_(container), idx_(idx) {}

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
        const std::vector<internal::function_node*>& container_;
        size_t idx_;
    };

    using value_type = function_descriptor;
    using reference = value_type&;
    using const_reference = const reference;
    using pointer = value_type*;
    using const_pointer = const pointer;
    using size_type = typename std::vector<internal::function_node*>::size_type;
    using difference_type = typename std::vector<internal::function_node*>::difference_type;
    using allocator_type = typename std::vector<internal::function_node*>::allocator_type;
    using const_iterator = const iterator;


    func_container(const std::vector<internal::function_node*>& nodes): nodes_(nodes) {}

    size_t size() const { return nodes_.size(); }

    function_descriptor operator[](size_t size) {
        return function_descriptor{*nodes_[size]};
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
    const std::vector<internal::function_node*>& nodes_;
};

class refl_info final {
public:
    refl_info(internal::type_node* node) : type_(node) {}

    std::string_view name() const { return type_->name; }

    operator bool() const { return type_ != nullptr; }

    auto funcs() const { return func_container{type_->funcs}; }

private:
    internal::type_node* type_;
};

}  // namespace drefl

}  // namespace mirrow