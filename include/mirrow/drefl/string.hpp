#pragma once

#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/value_kind.hpp"

namespace mirrow::drefl {

class any;

class string : public type {
public:
    enum class string_kind {
        Unknown,
        // ConstCharList
        String,
        StringView,
    };

    template <typename T>
    static string create() {
        return {get_kind<T>(), get_name<T>()};
    }

    auto string_kind() const noexcept { return kind_; }

    bool is_string() const noexcept { return kind_ == string_kind::String; }

    bool is_string_view() const noexcept {
        return kind_ == string_kind::StringView;
    }

    std::string get_str(const any&) const;
    std::string_view get_str_view(const any&) const;

    void set_value(any&, const std::string&) const;
    void set_value(any&, std::string_view&) const;

private:
    enum string_kind kind_;

    string(enum string_kind skind, const std::string& name);

    template <typename T>
    static auto get_kind() {
        if constexpr (std::is_same_v<std::string, T>) {
            return string_kind::String;
        }
        if constexpr (std::is_same_v<std::string_view, T>) {
            return string_kind::StringView;
        }
        return string_kind::Unknown;
    }

    template <typename T>
    static std::string get_name() {
        if constexpr (std::is_same_v<std::string, T>) {
            return "std::string";
        }
        if constexpr (std::is_same_v<std::string_view, T>) {
            return "std::string_view";
        }

        return "unknown-string-type";
    }
};

}  // namespace mirrow::drefl