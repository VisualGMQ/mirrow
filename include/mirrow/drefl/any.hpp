#pragma once

#include "mirrow/assert.hpp"
#include "mirrow/drefl/info_node.hpp"
#include "mirrow/drefl/type_info.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/misc.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace mirrow {

namespace drefl {

class any_methods;

class basic_any {
public:
    using storage_type = void*;

    virtual ~basic_any() = default;
    basic_any() = default;

    basic_any(internal::type_node* type, const any_methods* methods);

    void reset();

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

    std::optional<unsigned long long> try_cast_uintegral() const;
    std::optional<long long> try_cast_integral() const;
    std::optional<double> try_cast_floating_point() const;
    void deep_set(const basic_any& o);
    void push_back(const basic_any& data);
    size_t size();
    type_info elem_type();
    void travel_elements(const std::function<void(any&)>& func);
    void travel_elements_by_ref(const std::function<void(reference_any&)>& func);

    template <typename T>
    util::remove_cvref_t<T>* try_cast() {
        using type = util::remove_cvref_t<T>;
        if (internal::info_node<type>::resolve() != type_) {
            return nullptr;
        }
        return static_cast<type*>(instance_);
    }

    template <typename T>
    std::decay_t<T>* try_cast() const {
        using type = std::decay_t<T>;
        if (internal::info_node<type>::resolve() != type_) {
            return nullptr;
        }
        return static_cast<type*>(instance_);
    }

    bool has_value() const noexcept { return type_ && instance_; }

    operator bool() const noexcept { return has_value(); }

    type_info type() const { return ::mirrow::drefl::type_info{type_}; }

protected:
    const any_methods* methods_ = nullptr;
    void* instance_ = nullptr;
    internal::type_node* type_ = nullptr;
};

class reference_any;

/**
 * @brief similar as std::any, but use drefl type info
 */
class any final : public basic_any {
public:
    friend class reference_any;

    template <typename T>
    using enable_no_any_t =
        std::enable_if_t<!std::is_same_v<mirrow::util::remove_cvref_t<T>, any>,
                         T>;

    any() = default;

    template <typename T, typename... Args>
    any(std::in_place_type_t<T>, [[maybe_unused]] Args&&... args);

    template <typename T>
    any(const T& o);

    template <typename T>
    any(T&& o, enable_no_any_t<T>* = nullptr);

    explicit any(std::nullptr_t) {}

    any(const any& o);

    any(any&& o) { swap(*this, o); }

    ~any();

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
            o.methods_ = nullptr;
            o.type_ = nullptr;
        }
        return *this;
    }

private:
    friend void swap(any& a, any& b) {
        using std::swap;
        swap(a.type_, b.type_);
        swap(a.instance_, b.instance_);
        swap(a.methods_, b.methods_);
    }
};

class reference_any final : public basic_any {
public:
    template <typename T>
    using enable_not_any = std::enable_if_t<
        !std::is_same_v<reference_any, mirrow::util::remove_cvref_t<T>> &&
            !std::is_same_v<any, mirrow::util::remove_cvref_t<T>>,
        T>;

    reference_any() = default;

    template <typename T>
    reference_any(T& o, enable_not_any<T>* = nullptr);

    reference_any(any& o) : basic_any(o.type_, o.methods_) {
        instance_ = o.instance_;
    }

    explicit reference_any(std::nullptr_t) {}

    reference_any(const reference_any& o) : basic_any(o) {
        instance_ = o.instance_;
    }

    reference_any(reference_any&& o) { swap(*this, o); }

    reference_any& operator=(const reference_any& o) {
        if (&o != this) {
            reference_any dummy(o);
            swap(*this, dummy);
        }
        return *this;
    }

    reference_any& operator=(reference_any&& o) {
        if (&o != this) {
            swap(*this, o);
            o.instance_ = nullptr;
            o.type_ = nullptr;
            o.methods_ = nullptr;
        }
        return *this;
    }

private:
    friend void swap(reference_any& a, reference_any& b) {
        using std::swap;
        swap(a.type_, b.type_);
        swap(a.instance_, b.instance_);
        swap(a.methods_, b.methods_);
    }
};

class any_methods final {
public:
    using storage_type = void*;

    template <typename T>
    struct basic_op_traits {
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

        static void deep_set(storage_type& instance, const basic_any& o) {
            if constexpr (std::is_integral_v<type> ||
                          std::is_floating_point_v<type>) {
                if (o.type().is_fundamental()) {
                    auto fund = o.type().as_fundamental();
                    if (fund.is_integral()) {
                        if (fund.is_signed()) {
                            auto num = o.try_cast_integral();
                            if (num) {
                                *static_cast<type*>(instance) =
                                    static_cast<type>(num.value());
                            }
                        } else {
                            auto num = o.try_cast_uintegral();
                            if (num) {
                                *static_cast<type*>(instance) =
                                    static_cast<type>(num.value());
                            }
                        }
                    } else if (fund.is_floating_pointer()) {
                        auto num = o.try_cast_floating_point();
                        if (num) {
                            *static_cast<type*>(instance) =
                                static_cast<type>(num.value());
                        }
                    }
                }
            } else {
                auto data = o.try_cast<type>();
                if (data) {
                    *static_cast<type*>(instance) = *data;
                }
            }
        }
    };

    template <typename T>
    struct numeric_op_traits {
        using type = util::remove_cvref_t<T>;

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
    };

    template <typename T>
    struct container_op_traits {
        using type = util::remove_cvref_t<T>;

        static size_t size(storage_type& storage) {
            if constexpr (util::is_container_v<type>) {
                return static_cast<type*>(storage)->size();
            } else {
                return 0;
            }
        }

        // static reference_any at(storage_type& storage, size_t idx) {
        //     return reference_any{static_cast<T>(storage)->at(idx)};
        // }

        static type_info elem_type() {
            if constexpr (util::is_container_v<type>) {
                return type_info{internal::info_node<typename type::value_type>::type};
            } else {
                return type_info{nullptr};
            }
        }

        static void push_back(storage_type& storage, const basic_any& data) {
            if constexpr (util::is_vector_v<type>) {
                if (auto value = data.try_cast<typename type::value_type>();
                    value) {
                    static_cast<type*>(storage)->push_back(*value);
                }
            } else {
                MIRROW_LOG("container don't support push_back operation");
            }
        }

        static void travel_elements_by_ref(
            storage_type& data,
            const std::function<void(reference_any&)>& func) {
            if constexpr (util::is_container_v<type>) {
                type& container = *static_cast<type*>(data);
                for (auto& elem : container) {
                    reference_any a = elem;
                    if (func) {
                        func(a);
                    }
                }
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

    struct basic_operations {
        void* (*copy)(storage_type&, const void*);
        void* (*steal)(storage_type&, const void*);
        void (*destroy)(storage_type&);
        void (*deep_set)(storage_type&, const basic_any&);
    };

    struct numeric_cast_operations {
        std::optional<unsigned long long> (*try_cast_uintegral)(
            const storage_type&);
        std::optional<long long> (*try_cast_integral)(const storage_type&);
        std::optional<double> (*try_cast_floating_pointer)(const storage_type&);
    };

    struct container_operations {
        void (*travel)(storage_type&, const std::function<void(any&)>&);
        void (*travel_by_ref)(storage_type&,
                              const std::function<void(reference_any&)>&);
        size_t (*size)(storage_type&);
        // reference_any (*at)(storage_type&, size_t);
        void (*push_back)(storage_type&, const basic_any&);
        type_info (*elem_type)(void);
    };

    template <typename T>
    static const any_methods& instance() {
        static std::unique_ptr<any_methods> inst;

        if (!inst) {
            inst = std::make_unique<any_methods>();

            inst->basic_ops_.copy = &basic_op_traits<T>::copy;
            inst->basic_ops_.steal = &basic_op_traits<T>::move;
            inst->basic_ops_.destroy = &basic_op_traits<T>::destroy;
            inst->basic_ops_.deep_set = &basic_op_traits<T>::deep_set;

            inst->numeric_cast_.try_cast_uintegral =
                &numeric_op_traits<T>::try_cast_to_uint;
            inst->numeric_cast_.try_cast_integral =
                &numeric_op_traits<T>::try_cast_to_int;
            inst->numeric_cast_.try_cast_floating_pointer =
                &numeric_op_traits<T>::try_cast_to_double;

            if (util::is_container_v<util::remove_cvref_t<T>>) {
                inst->container_ops_.travel =
                    &container_op_traits<T>::travel_elements;
                inst->container_ops_.travel_by_ref =
                    &container_op_traits<T>::travel_elements_by_ref;
                inst->container_ops_.size = &container_op_traits<T>::size;
                // inst->container_ops_.at = &container_op_traits<T>::at;
                inst->container_ops_.push_back =
                    &container_op_traits<T>::push_back;
                inst->container_ops_.elem_type =
                    &container_op_traits<T>::elem_type;
            } else {
                inst->container_ops_.travel = nullptr;
                inst->container_ops_.travel_by_ref = nullptr;
                inst->container_ops_.size = nullptr;
                // inst->container_ops_.at = nullptr;
                inst->container_ops_.push_back = nullptr;
                inst->container_ops_.elem_type = nullptr;
            }
        }

        return *inst;
    }

    auto& basic() const { return basic_ops_; }

    auto& numeric_cast() const { return numeric_cast_; }

    auto& container() const { return container_ops_; }

private:
    basic_operations basic_ops_;
    numeric_cast_operations numeric_cast_;
    container_operations container_ops_;
};

template <typename T, typename... Args>
any::any(std::in_place_type_t<T>, [[maybe_unused]] Args&&... args)
    : basic_any(internal::info_node<T>::resolve(),
                &any_methods::instance<T>()) {
    any_methods::basic_op_traits<T>::create(instance_,
                                            std::forward<Args>(args)...);
}

template <typename T>
any::any(const T& o)
    : basic_any(internal::info_node<T>::resolve(),
                &any_methods::instance<T>()) {
    if (methods_ && methods_->basic().copy) {
        methods_->basic().copy(instance_, &o);
    }
}

template <typename T>
any::any(T&& o, enable_no_any_t<T>*)
    : basic_any(internal::info_node<T>::resolve(),
                &any_methods::instance<T>()) {
    if (methods_ && methods_->basic().steal) {
        methods_->basic().steal(instance_, &o);
    }
}


template <typename T>
reference_any::reference_any(T& o, reference_any::enable_not_any<T>*)
    : basic_any(internal::info_node<T>::resolve(),
                &any_methods::instance<T>()) {
    instance_ = (void*)&o;
}

}  // namespace drefl

}  // namespace mirrow