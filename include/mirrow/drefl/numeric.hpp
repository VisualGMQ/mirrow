#pragma once

#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/drefl/any.hpp"

namespace mirrow::drefl {

class numeric: public type {
public:
    template <typename>
    friend class numeric_factory;

    enum numeric_kind {
        Unknown,
        Char,
        Int,
        Short,
        Long,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Float,
        Double,
    };

    auto numeric_kind() const noexcept { return kind_; }

    numeric(value_kind value_kind, enum numeric_kind numeric_kind, const std::string& name)
        : type(value_kind, name), kind_(numeric_kind) {}

    void set_value(any&, long value) const;
    void set_value(any&, uint64_t value) const;
    void set_value(any&, double value) const;
    double get_value(const any&) const;

private:
    enum numeric_kind kind_;

    template <typename T>
    static numeric create() { return {get_kind_from_type<T>(), get_kind<T>(), get_name<T>()};}

    template <typename T>
    static enum numeric_kind get_kind() {
        if constexpr (std::is_same_v<int, T>) {
            return numeric_kind::Int;
        }
        if constexpr (std::is_same_v<char, T>) {
            return numeric_kind::Char;
        }
        if constexpr (std::is_same_v<short, T>) {
            return numeric_kind::Short;
        }
        if constexpr (std::is_same_v<long, T>) {
            return numeric_kind::Long;
        }
        if constexpr (std::is_same_v<uint8_t, T>) {
            return numeric_kind::Uint8;
        }
        if constexpr (std::is_same_v<uint16_t, T>) {
            return numeric_kind::Uint16;
        }
        if constexpr (std::is_same_v<uint32_t, T>) {
            return numeric_kind::Uint32;
        }
        if constexpr (std::is_same_v<uint64_t, T>) {
            return numeric_kind::Uint64;
        }
        if constexpr (std::is_same_v<float, T>) {
            return numeric_kind::Float;
        }
        if constexpr (std::is_same_v<double, T>) {
            return numeric_kind::Double;
        }

        return Unknown;
    }

    template <typename T>
    static std::string get_name() {
        if constexpr (std::is_same_v<int, T>) {
            return "int";
        }
        if constexpr (std::is_same_v<char, T>) {
            return "char";
        }
        if constexpr (std::is_same_v<short, T>) {
            return "short";
        }
        if constexpr (std::is_same_v<long, T>) {
            return "long";
        }
        if constexpr (std::is_same_v<uint8_t, T>) {
            return "uint8";
        }
        if constexpr (std::is_same_v<uint16_t, T>) {
            return "uint16";
        }
        if constexpr (std::is_same_v<uint32_t, T>) {
            return "uint32";
        }
        if constexpr (std::is_same_v<uint64_t, T>) {
            return "uint64";
        }
        if constexpr (std::is_same_v<float, T>) {
            return "float";
        }
        if constexpr (std::is_same_v<double, T>) {
            return "double";
        }

        return "unknown-numeric-type";
    }
};

}