#pragma once

#include "mirrow/drefl/enum.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/drefl/enum.hpp"
#include "mirrow/drefl/string.hpp"
#include "mirrow/drefl/qualifier.hpp"
#include "mirrow/util/misc.hpp"
#include "mirrow/util/variable_traits.hpp"
#include <string>


namespace mirrow::drefl {

class clazz;
class class_visitor;

namespace internal {

template <typename T>
using property_raw_t = util::remove_all_pointers_t<
    util::remove_cvref_t<typename util::variable_traits<T>::type>>;
}

class property : public type {
public:
    property(const std::string& name, const clazz* owner, qualifier q)
        : type(value_kind::Property, name), owner_(owner), qualifier_(q) {}

    virtual void visit(class_visitor*) = 0;
    // virtual any call(const any&) = 0;

    virtual ~property() = default;

    auto class_info() const noexcept { return owner_; }

    bool is_const_pointer() const noexcept {
        return static_cast<long>(qualifier_) &
               static_cast<long>(qualifier::ConstPointer);
    }

    bool is_const() const noexcept {
        return static_cast<long>(qualifier_) &
               static_cast<long>(qualifier::Const);
    }

    bool is_ref() const noexcept {
        return static_cast<long>(qualifier_) &
               static_cast<long>(qualifier::Ref);
    }

    bool is_pointer() const noexcept {
        return static_cast<long>(qualifier_) &
               static_cast<long>(qualifier::Pointer);
    }

    virtual const type* type_info() const noexcept = 0;

private:
    const clazz* owner_;
    qualifier qualifier_ = qualifier::None;
};

class string_property : public property {
public:
    using property::property;

    void visit(class_visitor* visitor) override;
};

template <typename T>
class string_property_impl : public string_property {
public:
    string_property_impl(const std::string& name, const clazz* owner,
                         qualifier q, T pointer)
        : string_property(name, owner, q),
          pointer_(pointer),
          type_info_(
              &string_factory<internal::property_raw_t<T>>::instance().info()) {
    }

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T pointer_;
    const string* type_info_;
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

template <typename T>
class boolean_property_impl : public boolean_property {
public:
    boolean_property_impl(const std::string& name, const clazz* owner,
                          qualifier q, T pointer)
        : boolean_property(name, owner, q), pointer_(pointer) {}

private:
    T pointer_;
};

class numeric_property : public property {
public:
    using property::property;

    void visit(class_visitor* visitor) override;
};

template <typename T>
class numeric_property_impl : public numeric_property {
public:
    numeric_property_impl(const std::string& name, const clazz* owner,
                          qualifier q, T pointer)
        : numeric_property(name, owner, q),
          pointer_(pointer),
          type_info_(&numeric_factory<internal::property_raw_t<T>>::instance()
                          .info()) {}

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T pointer_ = nullptr;
    const class numeric* type_info_;
};

class clazz_property : public property {
public:
    using property::property;

    void visit(class_visitor*) override;
};

template <typename T>
class clazz_property_impl : public clazz_property {
public:
    clazz_property_impl(const std::string& name, T accessor)
        : clazz_property(
              name,
              &class_factory<
                   util::remove_cvref_t<typename util::variable_traits<T>::clazz>>::instance()
                   .info(),
              get_qualifier<typename util::variable_traits<T>::type>()),
          accessor_(accessor),
          type_info_(
              &class_factory<internal::property_raw_t<T>>::instance().info()) {}

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T accessor_ = nullptr;
    const class clazz* type_info_;
};

class enum_property : public property {
public:
    explicit enum_property(const std::string& name, const clazz* owner,
                           qualifier q, const enum_info& enum_info)
        : property(name, owner, q) {}

    void visit(class_visitor* visitor) override;
};

template <typename T>
class enum_property_impl : public enum_property {
public:
    enum_property_impl(const std::string& name, const clazz* owner, qualifier q,
                       T pointer)
        : enum_property(name, owner, q),
          pointer_(pointer),
          type_info_(
              &enum_factory<internal::property_raw_t<T>>::instance().info()) {}

    const struct type* type_info() const noexcept override {
        return type_info_;
    }

private:
    T pointer_ = nullptr;
    const class enum_info* type_info_ = nullptr;
};

}  // namespace mirrow::drefl