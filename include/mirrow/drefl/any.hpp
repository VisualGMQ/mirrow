#pragma once

#include "mirrow/assert.hpp"
#include "mirrow/drefl/info_node.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/misc.hpp"

#include <type_traits>
#include <utility>
#include <memory>
#include <functional>

namespace mirrow {

namespace drefl {

namespace internal {

template <auto Func, size_t... Indices>
decltype(auto) invoke(any* args, std::index_sequence<Indices...>);

}  // namespace internal

/**
 * @brief similar as std::any, but use drefl type info
 */
class any final {
public:
    using storage_type = void*;

    any() = default;

    template <typename T, typename... Args>
    any(std::in_place_type_t<T>, [[maybe_unused]] Args&&... args)
        : copy_(&traits<T>::copy),
          move_(&traits<T>::move),
          destroy_(&traits<T>::destroy),
          type_(internal::info_node<T>::resolve()) {
        traits<T>::create(instance_, std::forward<Args>(args)...);
    }

    template <typename T>
    any(const T& o)
        : copy_(&traits<T>::copy),
          move_(&traits<T>::move),
          destroy_(&traits<T>::destroy),
          type_(internal::info_node<T>::resolve()) {
        copy_(instance_, &o);
    }

    template <typename T>
    any(T&& o)
        : copy_(&traits<T>::copy),
          move_(&traits<T>::move),
          destroy_(&traits<T>::destroy),
          type_(internal::info_node<T>::resolve()) {
        move_(instance_, &o);
    }

    any(const any& o)
        : copy_(o.copy_), move_(o.move_), destroy_(o.destroy_), type_(o.type_) {
        copy_(instance_, o.instance_);
    }

    any(any&& o) { swap(*this, o); }

    ~any() {
        if (destroy_) {
            destroy_(instance_);
        }
    }

    any& operator=(const any& o) {
        if (&o != this) {
            any dummy(o);
            swap(*this, dummy);
        }
        return *this;
    }

    any& operator=(any&& o) {
        if (&o != this) {
            swap(*this, o);
            o.instance_ = nullptr;
            o.move_ = nullptr;
            o.copy_ = nullptr;
            o.destroy_ = nullptr;
        }
        return *this;
    }

    template <typename T>
    auto& cast() {
        using type = util::remove_cvref_t<T>;
        return *static_cast<type*>(instance_);
    }

    template <typename T>
    bool can_cast() const {
        return internal::info_node<util::remove_cvref_t<T>>::resolve() == type_;
    }

    template <typename T>
    util::remove_cvref_t<T>* try_cast() {
        using type = util::remove_cvref_t<T>;
        if (internal::info_node<type>::resolve() != type_) {
            return nullptr;
        }
        return static_cast<type*>(instance_);
    }

    bool has_value() const noexcept { return type_ && instance_; }

    operator bool() const noexcept { return has_value(); }

    internal::type_node* const type() const { return type_; }

private:
    template <typename T>
    struct traits {
        using type = util::remove_cvref_t<T>;

        template <typename... Args>
        static void* create(storage_type& storage, Args&&... args) {
            auto instance = std::make_unique<type>(std::forward<Args>(args)...);
            new (&storage) type* {instance.get()};
            return instance.release();
        }

        static void* copy(storage_type& storage, const void* from) {
            auto instance =
                std::make_unique<type>(*static_cast<const type*>(from));
            new (&storage) type* {instance.get()};
            return instance.release();
        };

        static void destroy(storage_type& obj) {
            auto actual = static_cast<type*>(obj);
            delete actual;
            obj = nullptr;
        }

        static void* move(storage_type& to, const void* from) {
            auto instance = std::make_unique<type>(
                std::move(*static_cast<const type*>(from)));
            new (&to) type* {instance.get()};
            return instance.release();
        }
    };

    using copy_fn_type = void* (*)(storage_type&, const void*);
    using move_fn_type = void* (*)(storage_type&, const void*);
    using destroy_fn_type = void (*)(storage_type&);

    copy_fn_type copy_ = nullptr;
    move_fn_type move_ = nullptr;
    destroy_fn_type destroy_ = nullptr;
    void* instance_ = nullptr;
    internal::type_node* type_ = nullptr;

    friend void swap(any& a, any& b) {
        using std::swap;
        swap(a.type_, b.type_);
        swap(a.instance_, b.instance_);
    }
};


namespace internal {

template <typename T>
struct show_tpl;

template <auto Func, size_t... Indices>
decltype(auto) invoke(any* args, std::index_sequence<Indices...>) {
    using fn_traits = util::function_pointer_traits<Func>;

    static_assert(fn_traits::args_with_class::size == sizeof...(Indices),
                  "the number of function call arguments is inconsistent");

    return std::invoke(
        Func, ((args + Indices)
                  ->cast<util::remove_cvref_t<util::list_element_t<
                      typename fn_traits::args_with_class, Indices>>>())...);
}

}  // namespace internal

}  // namespace drefl

}  // namespace mirrow