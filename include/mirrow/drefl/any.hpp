#pragma once

#include "mirrow/drefl/exception.hpp"
#include "mirrow/drefl/operation_traits.hpp"
#include "mirrow/util/misc.hpp"


namespace mirrow::drefl {

struct type;

class any final {
public:
    friend class clazz;

    template <typename T>
    friend any any_make_constref(const T&) noexcept;

    template <typename T>
    friend any any_make_ref(T&) noexcept;

    template <typename T>
    friend any any_make_copy(T&& value) noexcept(
        std::is_rvalue_reference_v<T&&>
            ? std::is_nothrow_move_constructible_v<util::remove_cvref_t<T>>
            : std::is_nothrow_copy_constructible_v<util::remove_cvref_t<T>>);

    template <typename T>
    friend T* try_cast(any&);

    template <typename T>
    friend const T* try_cast_const(const any&);

    enum class access_type {
        Null,
        ConstRef,
        Ref,
        Copy,
    };

    auto access_type() const noexcept { return access_; }

    any() = default;

    any(enum access_type access, void* payload, const type_operations* operations,
        const type* typeinfo)
        : access_(access),
          payload_(payload),
          operations_(operations),
          type_(typeinfo) {}

    any(const any&);
    any(any&& o);
    any& operator=(const any&);
    any& operator=(any&& o);

    ~any();

    any constref() noexcept {
        return {access_type::ConstRef, payload_, operations_, type_};
    }

    any ref() {
        if (access_type() == access_type::ConstRef) {
            throw bad_any_access("can't make_ref from const& any");
        }
        return {access_type::Ref, payload_, operations_, type_};
    }

    any copy() {
        void* elem = operations_->copy_construct(payload_);
        return {access_type::Copy, elem, operations_, type_};
    }

    void copy_assign(any& o) {
        if (o.type_info() == type_info()) {
            operations_->copy_assignment(payload_, o.payload_);
        } else {
            MIRROW_LOG("can't copy assign between two different types");
        }
    }

    void steal_assign(any&& o) {
        if (o.type_info() == type_info()) {
            operations_->steal_assignment(payload_, o.payload_);
        } else {
            MIRROW_LOG("can't copy assign between two different types");
        }
    }

    any steal() {
        auto access = access_;
        auto payload = operations_->steal_construct(payload_);
        auto operations = operations_;
        auto type = type_;

        access_ = access_type::Null;
        if (payload_ && access_ == access_type::Copy) {
            operations_->destroy(payload_);
        }
        payload_ = nullptr;
        type_ = nullptr;

        return {access, payload, operations, type};
    }

    const type* type_info() const noexcept { return type_; }

    bool has_value() const noexcept { return access_ != access_type::Null && payload_; }

    bool is_ref() const noexcept {
        return access_ == access_type::Ref;
    }

    bool is_constref() const noexcept {
        return access_ == access_type::ConstRef;
    }

    bool is_copy() const noexcept {
        return access_ == access_type::Copy;
    }

    bool is_null() const noexcept {
        return access_ == access_type::Null;
    }

    void* payload() noexcept { return payload_; }
    const void* payload() const noexcept { return payload_; }

    /**
     * @brief release payload
     */
    void* release() {
        auto payload = payload_;
        payload_ = nullptr;
        access_ = access_type::Null;
        return payload;
    }

private:
    enum access_type access_ = access_type::Null;
    void* payload_ = nullptr;
    const type_operations* operations_ = &type_operations::null;
    const type* type_ = nullptr;
};

}  // namespace mirrow::drefl