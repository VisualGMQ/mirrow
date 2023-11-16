#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/array.hpp"
#include "mirrow/drefl/bool.hpp"
#include "mirrow/drefl/class.hpp"
#include "mirrow/drefl/enum.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/numeric.hpp"
#include "mirrow/drefl/pointer.hpp"
#include "mirrow/drefl/raw_type.hpp"
#include "mirrow/drefl/string.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/misc.hpp"
#include "mirrow/util/variable_traits.hpp"
#include <memory>
#include <type_traits>


namespace mirrow::drefl {

namespace internal {

template <typename T>
constexpr bool is_array_v = std::is_array_v<T> || util::is_std_array_v<T> ||
                            util::is_vector_v<T> || util::is_std_list_v<T>;

template <typename T>
constexpr bool is_string_v =
    std::is_same_v<std::string, T> || std::is_same_v<std::string_view, T>;
}

class clazz;
class class_visitor;

template <typename>
class class_factory;

class boolean_factory final {
public:
    static auto& instance() noexcept {
        static boolean_factory inst;
        return inst;
    }

    auto& info() const noexcept { return info_; }

private:
    boolean info_;
};

class property : public type {
public:
    property(const std::string& name, const clazz* owner, qualifier q)
        : type(value_kind::Property, name), owner_(owner), qualifier_(q) {}

    virtual void visit(class_visitor*) = 0;
    virtual any call_const(const any&) const = 0;
    virtual any call(any&) const = 0;

    virtual ~property() = default;

    auto class_info() const noexcept { return owner_; }

    bool is_const() const noexcept {
        return static_cast<long>(qualifier_) &
               static_cast<long>(qualifier::Const);
    }

    bool is_ref() const noexcept {
        return static_cast<long>(qualifier_) &
               static_cast<long>(qualifier::Ref);
    }

    virtual const type* type_info() const noexcept = 0;

    const clazz* owner() const noexcept { return owner_; }

private:
    const clazz* owner_;
    qualifier qualifier_ = qualifier::None;
};

class string_property : public property {
public:
    using property::property;

    void visit(class_visitor* visitor) override;
};

class boolean_property : public property {
public:
    boolean_property(const std::string& name, const clazz* owner, qualifier q)
        : property(name, owner, q),
          type_info_(&boolean_factory::instance().info()) {}

    void visit(class_visitor* visitor) override;

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    const boolean* type_info_;
};

class pointer_property : public property {
public:
    pointer_property(const std::string& name, const clazz* owner, qualifier q,
                     const type* pointer_type)
        : property(name, owner, q), pointer_type_(pointer_type) {}

    void visit(class_visitor* visitor) override;

    const type* pointer_type() const noexcept { return pointer_type_; }

private:
    const type* pointer_type_;
};

class numeric_property : public property {
public:
    using property::property;

    void visit(class_visitor* visitor) override;
};

class clazz_property : public property {
public:
    using property::property;

    void visit(class_visitor*) override;
};

class enum_property : public property {
public:
    explicit enum_property(const std::string& name, const clazz* owner,
                           qualifier q)
        : property(name, owner, q) {}

    void visit(class_visitor* visitor) override;
};

class array_property : public property {
public:
    explicit array_property(const std::string& name, const clazz* owner,
                            qualifier q)
        : property(name, owner, q) {}

    void visit(class_visitor* visitor) override;
};

template <typename T>
any call_property_const(const any& a, T accessor, const type* owner) {
    if (owner != a.type_info() || a.is_null()) {
        return {};
    }

    using traits = util::variable_traits<T>;
    using clazz = typename traits::clazz;
    using type = util::remove_cvref_t<typename traits::type>;

    auto& value = ((const clazz*)(a.payload()))->*accessor;

    auto& operations = type_operation_traits<type>::get_operations();
    auto info = factory<type>::info();
    if (!info) {
        info = &class_factory<type>::instance().info();
    }

    return {any::access_type::ConstRef, (void*)&value, &operations, info};
}

template <typename T>
any call_property(any& a, T accessor, const type* owner) {
    if (owner != a.type_info() || a.is_null()) {
        return {};
    }

    using traits = util::variable_traits<T>;
    using clazz = typename traits::clazz;
    using type = util::remove_cvref_t<typename traits::type>;

    auto& value = ((clazz*)a.payload())->*accessor;
    auto& operations = type_operation_traits<type>::get_operations();
    auto info = factory<type>::info();
    if (!info) {
        info = &class_factory<type>::instance().info();
    }

    if (a.is_constref()) {
        return {any::access_type::ConstRef, (void*)&value, &operations, info};
    } else {
        return {any::access_type::Ref, (void*)&value, &operations, info};
    }
}

template <typename T>
class string_property_impl : public string_property {
public:
    string_property_impl(const std::string& name, const clazz* owner,
                         qualifier q, T pointer)
        : string_property(name, owner, q),
          pointer_(pointer),
          type_info_(&string_factory<util::remove_cvref_t<
                          typename util::variable_traits<T>::type>>::instance()
                          .info()) {}

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T pointer_;
    const string* type_info_;
};

template <typename T>
class boolean_property_impl : public boolean_property {
public:
    boolean_property_impl(const std::string& name, const clazz* owner,
                          qualifier q, T pointer)
        : boolean_property(name, owner, q), pointer_(pointer) {}

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

private:
    T pointer_;
};

template <typename T>
class numeric_property_impl : public numeric_property {
public:
    numeric_property_impl(const std::string& name, const clazz* owner,
                          qualifier q, T pointer)
        : numeric_property(name, owner, q),
          pointer_(pointer),
          type_info_(&numeric_factory<util::remove_cvref_t<
                          typename util::variable_traits<T>::type>>::instance()
                          .info()) {}

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T pointer_ = nullptr;
    const class numeric* type_info_;
};

template <typename T>
class pointer_property_impl : public pointer_property {
public:
    pointer_property_impl(const std::string& name, const clazz* owner,
                          qualifier q, const type* pointer_type, T pointer)
        : pointer_property(name, owner, q, pointer_type),
          pointer_(pointer),
          type_info_(&numeric_factory<util::remove_cvref_t<
                          typename util::variable_traits<T>::type>>::instance()
                          .info()) {}

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T pointer_ = nullptr;
    const class numeric* type_info_;
};

template <typename T>
class clazz_property_impl : public clazz_property {
public:
    clazz_property_impl(const std::string& name, T accessor)
        : clazz_property(
              name,
              &class_factory<util::remove_cvref_t<
                   typename util::variable_traits<T>::clazz>>::instance()
                   .info(),
              get_qualifier<typename util::variable_traits<T>::type>()),
          pointer_(accessor),
          type_info_(&class_factory<util::remove_cvref_t<
                          typename util::variable_traits<T>::type>>::instance()
                          .info()) {}

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

private:
    T pointer_ = nullptr;
    const class clazz* type_info_;
};

template <typename T>
class enum_property_impl : public enum_property {
public:
    enum_property_impl(const std::string& name, const clazz* owner, qualifier q,
                       T pointer)
        : enum_property(name, owner, q),
          pointer_(pointer),
          type_info_(&enum_factory<util::remove_cvref_t<
                          typename util::variable_traits<T>::type>>::instance()
                          .info()) {}

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

private:
    T pointer_ = nullptr;
    const class enum_info* type_info_ = nullptr;
};

template <typename T>
class array_property_impl : public array_property {
public:
    array_property_impl(const std::string& name, const clazz* owner,
                        qualifier q, T pointer)
        : array_property(name, owner, q),
          pointer_(pointer),
          type_info_(&array_factory<util::remove_cvref_t<
                          typename util::variable_traits<T>::type>>::instance()
                          .info()) {}

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

    any call_const(const any& a) const override {
        return call_property_const(a, pointer_, this->owner());
    }

    any call(any& a) const override {
        return call_property(a, pointer_, this->owner());
    }

private:
    T pointer_ = nullptr;
    const class array* type_info_ = nullptr;
};

class enum_property_factory final {
public:
    template <typename T>
    enum_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;
        using enum_type = util::remove_cvref_t<var_type>;

        static_assert(std::is_enum_v<enum_type>);

        auto& enum_info = enum_factory<enum_type>::instance().info();

        property_ = std::make_shared<enum_property_impl<T>>(
            name, get_qualifier<var_type>(), enum_info, accessor);
    }

    auto& get() const noexcept { return property_; }

private:
    std::shared_ptr<enum_property> property_;
};

class numeric_property_factory final {
public:
    template <typename T>
    numeric_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;

        property_ = std::make_shared<numeric_property_impl<T>>(
            name, &class_factory<typename traits::clazz>::instance().info(),
            get_qualifier<var_type>(), accessor);
    }

    auto& get() const noexcept { return property_; }

private:
    std::shared_ptr<numeric_property> property_;
};

class string_property_factory final {
public:
    template <typename T>
    string_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;

        property_ = std::make_shared<string_property_impl<T>>(
            name, &class_factory<typename traits::clazz>::instance().info(),
            get_qualifier<var_type>(), accessor);
    }

    auto& get() const noexcept { return property_; }

private:
    std::shared_ptr<string_property> property_;
};

class boolean_property_factory final {
public:
    template <typename T>
    boolean_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;

        property_ = std::make_shared<boolean_property_impl<T>>(
            name, &class_factory<typename traits::clazz>::instance().info(),
            get_qualifier<var_type>(), accessor);
    }

    auto& get() const noexcept { return property_; }

private:
    std::shared_ptr<boolean_property> property_;
};

class pointer_property_factory final {
public:
    template <typename T>
    pointer_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;

        property_ = std::make_shared<pointer_property_impl<T>>(
            name, &class_factory<typename traits::clazz>::instance().info(),
            get_qualifier<var_type>(), factory<util::remove_cvref_t<T>>::info(),
            accessor);
    }

    auto& get() const noexcept { return property_; }

private:
    std::shared_ptr<pointer_property> property_;
};

class array_property_factory final {
public:
    template <typename T>
    array_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;

        property_ = std::make_shared<array_property_impl<T>>(
            name, &class_factory<typename traits::clazz>::instance().info(),
            get_qualifier<var_type>(), accessor);
    }

    auto& get() const noexcept { return property_; }

private:
    std::shared_ptr<array_property> property_;
};



class class_property_factory final {
public:
    template <typename T>
    class_property_factory(const std::string& name, T accessor) {
        using traits = util::variable_traits<T>;
        using var_type = typename traits::type;

        property_ = std::make_shared<clazz_property_impl<T>>(name, accessor);
    }

    auto& get() const { return property_; }

private:
    std::shared_ptr<clazz_property> property_;
};

class property_factory final {
public:
    template <typename T>
    std::shared_ptr<property> create(const std::string& name, T accessor) {
        if constexpr (util::is_function_v<T>) {
            // TODO: use function_factory here
            return nullptr;
        } else {
            using traits = util::variable_traits<T>;
            using type = util::remove_cvref_t<typename traits::type>;

            if constexpr (std::is_pointer_v<type>) {
                return pointer_property_factory{name, accessor}.get();
            } else if constexpr (std::is_same_v<bool, type>) {
                return boolean_property_factory{name, accessor}.get();
            } else if constexpr (std::is_enum_v<type>) {
                return enum_property_factory{name, accessor}.get();
            } else if constexpr (std::is_fundamental_v<type>) {
                return numeric_property_factory{name, accessor}.get();
            } else if constexpr (internal::is_string_v<type>) {
                return string_property_factory{name, accessor}.get();
            } else if constexpr (internal::is_array_v<type>) {
                return array_property_factory{name, accessor}.get();
            } else {
                return class_property_factory{name, accessor}.get();
            }
        }
    }
};

template <typename T>
class enum_factory final {
public:
    static_assert(std::is_enum_v<T>);

    template <typename>
    friend class enum_factory;

    static enum_factory& instance() noexcept {
        static enum_factory inst;
        return inst;
    }

    auto& regist(const std::string& name) noexcept {
        enum_info_.name_ = name;

        return *this;
    }

    template <typename U>
    auto& add(const std::string& name, U value) noexcept {
        enum_info_.add(name, value);

        return *this;
    }

    auto& info() const noexcept { return enum_info_; }

    void unregist() noexcept {
        enum_info_.items_.clear();
        enum_info_.name_.clear();
    }

    bool has_registed() const noexcept { return !enum_info_.name_.empty(); }

private:
    class enum_info enum_info_;
};

template <typename T>
class numeric_factory final {
public:
    static auto& instance() noexcept {
        static numeric_factory inst{numeric::create<T>()};
        return inst;
    }

    auto& info() const noexcept { return info_; }

private:
    numeric_factory(numeric s) : info_(s) {}

    numeric info_;
};

template <typename T>
class string_factory final {
public:
    static auto& instance() noexcept {
        static string_factory inst{string::create<T>()};
        return inst;
    }

    auto& info() const noexcept { return info_; }

private:
    string_factory(const string& info) : info_(info) {}

    string info_;
};

template <typename T>
class pointer_factory final {
public:
    static auto& instance() noexcept;

    auto& info() const noexcept { return info_; }

private:
    pointer_factory(const pointer& p) : info_{p} {}

    pointer info_;
};

template <typename T>
class array_factory final {
public:
    static auto& instance() noexcept;

    auto& info() const noexcept { return info_; }

private:
    array_factory(const array& a) : info_{a} {}

    array info_;
};

template <typename T>
class factory final {
public:
    static const type* info() noexcept {
        if constexpr (std::is_pointer_v<T>) {
            return &pointer_factory<T>::instance().info();
        }
        if constexpr (std::is_same_v<bool, T>) {
            return &boolean_factory::instance().info();
        }
        if constexpr (internal::is_string_v<T>) {
            return &string_factory<T>::instance().info();
        }
        if constexpr (std::is_enum_v<T>) {
            return &enum_factory<T>::instance().info();
        }
        if constexpr (internal::is_array_v<T>) {
            return &array_factory<T>::instance().info();
        }
        if constexpr (std::is_fundamental_v<T>) {
            return &numeric_factory<T>::instance().info();
        }

        return nullptr;
    }
};

template <typename T>
class class_factory final {
public:
    static auto& instance() noexcept {
        static class_factory inst;
        return inst;
    }

    auto& regist(const std::string& name) {
        info_.name_ = name;

        return *this;
    }

    template <typename U>
    auto& property(const std::string& name, U accessor) {
        using traits = util::variable_traits<U>;
        using type = typename traits::type;

        info_.properties_.emplace_back(
            property_factory{}.create(name, accessor));

        return *this;
    }

    auto& info() const noexcept { return info_; }

private:
    clazz info_;
};

template <typename T>
const type* typeinfo() {
    auto t = factory<T>::info();

    if (!t) {
        t = &class_factory<T>::instance().info();
    }

    return t;
}

template <typename T>
auto& pointer_factory<T>::instance() noexcept {
    static pointer_factory inst{pointer::create<T>(typeinfo<raw_type_t<T>>())};
    return inst;
}

namespace internal {

template <typename T>
struct array_element_type {
    using type = typename T::value_type;
};

template <typename T, size_t N>
struct array_element_type<T[N]> {
    using type = T;
};

};  // namespace internal

template <typename T>
auto& array_factory<T>::instance() noexcept {
    static array_factory inst{
        array::create<T>(typeinfo<util::array_element_t<T>>())};
    return inst;
}

}  // namespace mirrow::drefl