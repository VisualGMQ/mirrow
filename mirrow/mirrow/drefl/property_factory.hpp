#pragma once

#include "mirrow/drefl/class.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/property.hpp"
#include "mirrow/util/function_traits.hpp"
#include "mirrow/util/misc.hpp"
#include "mirrow/util/variable_traits.hpp"
#include <memory>
#include <type_traits>


namespace mirrow::drefl {

template <typename>
class class_factory;

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

            if constexpr (std::is_same_v<bool, type>) {
                return boolean_property_factory{name, accessor}.get();
            } else if constexpr (std::is_enum_v<type>) {
                return enum_property_factory{name, accessor}.get();
            } else if constexpr (std::is_fundamental_v<type>) {
                return numeric_property_factory{name, accessor}.get();
            } else if constexpr (std::is_same_v<std::string, type> ||
                                 std::is_same_v<std::string_view, type>) {
                return string_property_factory{name, accessor}.get();
            } else {
                return class_property_factory{name, accessor}.get();
            }
        }
    }
};

}  // namespace mirrow::drefl