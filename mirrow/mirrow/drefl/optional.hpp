#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/operation_traits.hpp"
#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/value_kind.hpp"


namespace mirrow::drefl {

class optional final : public type {
public:
    template <typename T>
    friend class optional_factory;

    using set_value_fn = void(const void*, void*);
    using set_inner_value_fn = void(const void*, void*);
    using get_value_fn = void*(void*);
    using has_value_fn = bool(const void*);

    struct operations final {
        set_value_fn* set_value_ = nullptr;
        get_value_fn* get_value_ = nullptr;
        has_value_fn* has_value_ = nullptr;
        set_inner_value_fn* set_inner_value_ = nullptr;
    };

    template <typename T>
    struct traits {
        using value_type = typename T::value_type;

        static void set_inner_value(const void* src, void* dst) {
            *(T*)(dst) = *(value_type*)src;
        }

        static void set_value(const void* src, void* dst) {
            *(T*)(dst) = *(T*)src;
        }

        static void* get_value(void* value) {
            return &((T*)value)->value();
        }

        static bool has_value(const void* value) {
            return ((const T*)(value))->has_value();
        }

        static const operations& get_operations() {
            using type = traits<T>;
            static operations ops = {type::set_value, type::get_value,
                                     type::has_value, type::set_inner_value};
            return ops;
        }
    };

    optional(const type* elem_type, const std::string& name,
                      const operations& ops, const type_operations& elem_ops)
        : type{value_kind::Optional, name, nullptr},
          elem_type_(elem_type),
          ops_{&ops},
          elem_operations_(&elem_ops) {}

    template <typename T>
    static optional create(const type* elem_type) {
        return {elem_type, "std::optional<" + elem_type->name() + ">",
                traits<T>::get_operations(),
                type_operation_traits<typename T::value_type>::get_operations()};
    }

    const type* elem_type() const noexcept { return elem_type_; }

    any get_value(any& value) const {
        void* elem = ops_->get_value_(value.payload());
        return {value.is_constref() ? any::access_type::ConstRef
                                    : any::access_type::Ref,
                elem, elem_operations_, elem_type_};
    }

    any get_value_const(const any& value) const {
        void* elem = ops_->get_value_((void*)value.payload());
        return {any::access_type::ConstRef,
                elem, elem_operations_, elem_type_};
    }

    void set_value(const any& src, any& value) const {
        if (src.type_info() == this) {
            ops_->set_value_(src.payload(), value.payload());
        } else {
            MIRROW_LOG("can't set std::optional value due to type incorrect");
        }
    }

    void set_inner_value(const any& src, any& value) const {
        if (src.type_info() == elem_type_) {
            ops_->set_inner_value_(src.payload(), value.payload());
        } else {
            MIRROW_LOG("can't set std::optional value due to type incorrect");
        }
    }

    bool has_value(const any& value) const noexcept {
        return ops_->has_value_(value.payload());
    }

private:
    const type* elem_type_;
    const operations* ops_;
    const type_operations* elem_operations_;
};

}  // namespace mirrow::drefl