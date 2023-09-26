#pragma once

#include "mirrow/drefl/info_node.hpp"

#include <vector>

namespace mirrow {

namespace drefl {

class ctor_descriptor final {
public:
    friend any invoke_by_any(const ctor_descriptor& var, basic_any* args);

    using type = internal::ctor_node;

    explicit ctor_descriptor(const type& n) : node_(&n) {}

    auto parent() const { return node_->parent; }

    const type* node() const { return node_; }

    auto& params() const {
        return node_->params;
    }

private:
    const type* node_;
};

class function_descriptor final {
public:
    using type = internal::function_node;

    explicit function_descriptor(const type& n) : node_(&n) {}

    std::string_view name() const { return node_->name; }

    auto parent() const { return node_->parent; }

    const type* node() const { return node_; }

private:
    const type* node_;
};

class variable_descriptor final {
public:
    friend any invoke_by_any(const variable_descriptor& var, basic_any* args);

    template <typename... Args>
    friend any invoke(const function_descriptor& func, Args&&... args);

    using type = internal::variable_node;

    explicit variable_descriptor(const type& n) : node_(&n) {}

    std::string_view name() const { return node_->name; }

    auto parent() const { return node_->parent; }

    const type* node() const { return node_; }

    auto raw_type() const { return node_->type->raw_type;}

private:
    const type* node_;
};

template <typename T, typename Descriptor>
class field_container final {
public:
    class iterator final {
    public:
        iterator(const std::vector<T*>& container, size_t idx)
            : container_(container), idx_(idx) {}

        bool operator==(const iterator& o) const {
            return &o.container_ == &container_ && o.idx_ == idx_;
        }

        bool operator!=(const iterator& o) const { return !(*this == o); }

        iterator& operator+=(size_t step) { return idx_ += step, *this; }

        iterator& operator-=(size_t step) { return idx_ -= step, *this; }

        iterator operator+(size_t step) {
            iterator tmp = *this;
            return tmp += step;
        }

        iterator operator-(size_t step) {
            iterator tmp = *this;
            return tmp -= step;
        }

        iterator& operator++() { return *this += 1, *this; }

        iterator operator++(int) {
            iterator tmp = *this;
            return ++(*this), tmp;
        }

        iterator& operator--() { return *this -= 1, *this; }

        iterator operator--(int) {
            iterator tmp = *this;
            return --(*this), tmp;
        }

        Descriptor operator*() { return Descriptor{*container_[idx_]}; }

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

    field_container(const std::vector<T*>& nodes) : nodes_(nodes) {}

    size_t size() const { return nodes_.size(); }

    Descriptor operator[](size_t size) { return Descriptor{*nodes_[size]}; }

    const_iterator begin() const { return iterator{nodes_, 0}; }

    const_iterator end() const { return iterator{nodes_, nodes_.size()}; }

    iterator begin() {
        return const_cast<iterator&&>(std::as_const(*this).begin());
    }

    iterator end() {
        return const_cast<iterator&&>(std::as_const(*this).end());
    }

private:
    const std::vector<T*>& nodes_;
};

using function_container =
    field_container<typename function_descriptor::type, function_descriptor>;
using variable_container =
    field_container<typename variable_descriptor::type, variable_descriptor>;

using ctor_container = field_container<typename ctor_descriptor::type, ctor_descriptor>;

}  // namespace drefl

}  // namespace mirrow