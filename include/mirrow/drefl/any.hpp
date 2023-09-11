#pragma once

#include "mirrow/assert.hpp"
#include "mirrow/drefl/info_node.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/misc.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

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
          try_cast_to_uint_(&traits<T>::try_cast_to_uint),
          try_cast_to_int_(&traits<T>::try_cast_to_int),
          try_cast_to_double_(&traits<T>::try_cast_to_double),
          travel_elements_(&traits<T>::travel_elements),
          type_(internal::info_node<T>::resolve()) {
        traits<T>::create(instance_, std::forward<Args>(args)...);
    }

    template <typename T>
    any(const T& o)
        : copy_(&traits<T>::copy),
          move_(&traits<T>::move),
          destroy_(&traits<T>::destroy),
          try_cast_to_uint_(&traits<T>::try_cast_to_uint),
          try_cast_to_int_(&traits<T>::try_cast_to_int),
          try_cast_to_double_(&traits<T>::try_cast_to_double),
          travel_elements_(&traits<T>::travel_elements),
          type_(internal::info_node<T>::resolve()) {
        copy_(instance_, &o);
    }

    template <typename T>
    any(std::enable_if_t<!std::is_same_v<T, any>>&& o)
        : copy_(&traits<T>::copy),
          move_(&traits<T>::move),
          destroy_(&traits<T>::destroy),
          try_cast_to_uint_(&traits<T>::try_cast_to_uint),
          try_cast_to_int_(&traits<T>::try_cast_to_int),
          try_cast_to_double_(&traits<T>::try_cast_to_double),
          travel_elements_(&traits<T>::travel_elements),
          type_(internal::info_node<T>::resolve()) {
        move_(instance_, &o);
    }

    any(const any& o)
        : copy_(o.copy_),
          move_(o.move_),
          destroy_(o.destroy_),
          try_cast_to_uint_(o.try_cast_to_uint_),
          try_cast_to_int_(o.try_cast_to_int_),
          try_cast_to_double_(o.try_cast_to_double_),
          travel_elements_(o.travel_elements_),
          type_(o.type_) {
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
        using type = std::decay_t<T>;
        return *static_cast<type*>(instance_);
    }

    template <typename T>
    bool can_cast() const {
        return internal::info_node<std::decay_t<T>>::resolve() == type_;
    }

    auto get_category() const { return type_->category; }

    auto try_cast_uintegral() { return try_cast_to_uint_(instance_); }

    auto try_cast_integral() { return try_cast_to_int_(instance_); }

    auto try_cast_floating_point() { return try_cast_to_double_(instance_); }

    void travel_elements(const std::function<void(any&)>& func) {
        travel_elements_(instance_, func);
    }

    template <typename T>
    std::decay_t<T>* try_cast() {
        using type = std::decay_t<T>;
        if (internal::info_node<type>::resolve() != type_) {
            return nullptr;
        }
        return static_cast<type*>(instance_);
    }

    bool has_value() const noexcept { return type_ && instance_; }

    operator bool() const noexcept { return has_value(); }

    internal::type_node* const type_info() const { return type_; }

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

        static std::optional<unsigned long long> try_cast_to_uint(
            const storage_type& data) {
            if constexpr (std::is_integral_v<type> &&
                          std::is_unsigned_v<type>) {
                return *static_cast<const type*>(data);
            } else {
                return std::nullopt;
            }
        }

        static std::optional<long long> try_cast_to_int(
            const storage_type& data) {
            if constexpr (std::is_integral_v<type> && std::is_signed_v<type>) {
                return *static_cast<const type*>(data);
            } else {
                return std::nullopt;
            }
        }

        static std::optional<double> try_cast_to_double(
            const storage_type& data) {
            if constexpr (std::is_floating_point_v<type>) {
                return *static_cast<const type*>(data);
            } else {
                return std::nullopt;
            }
        }

        static void travel_elements(storage_type& data,
                                    const std::function<void(any&)>& func) {
            if constexpr (util::is_container_v<type>) {
                type& container = *static_cast<type*>(data);
                for (auto& elem : container) {
                    any a = elem;
                    if (func) {
                        func(a);
                    }
                }
            }
        }
    };

    using copy_fn_type = void* (*)(storage_type&, const void*);
    using move_fn_type = void* (*)(storage_type&, const void*);
    using destroy_fn_type = void (*)(storage_type&);
    using try_cast_to_uint_fn_type =
        std::optional<unsigned long long> (*)(const storage_type&);
    using try_cast_to_int_fn_type =
        std::optional<long long> (*)(const storage_type&);
    using try_cast_to_double_fn_type =
        std::optional<double> (*)(const storage_type&);
    using travel_elements_fn_type = void (*)(storage_type&,
                                             const std::function<void(any&)>&);

    copy_fn_type copy_ = nullptr;
    move_fn_type move_ = nullptr;
    destroy_fn_type destroy_ = nullptr;
    try_cast_to_uint_fn_type try_cast_to_uint_ = nullptr;
    try_cast_to_int_fn_type try_cast_to_int_ = nullptr;
    try_cast_to_double_fn_type try_cast_to_double_ = nullptr;
    travel_elements_fn_type travel_elements_ = nullptr;

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