#pragma once

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/array_operations.hpp"
#include "mirrow/drefl/operation_traits.hpp"
#include "mirrow/drefl/type.hpp"
#include "mirrow/util/misc.hpp"
#include <iterator>

namespace mirrow::drefl {

struct type_operations;

class array : public type {
public:
    enum class array_type {
        Static,
        Dynamic,
    };

    enum class addressing_type {
        Random,   // like std::vector, std::array
        Forward,  // like std::list
    };

    template <typename T>
    static array create(const type* elem_type) {
        return {
            get_name<T>(),
            get_arr_type<T>(),
            get_addr_type<T>(),
            elem_type,
            &array_operation_traits<T>::get_operations(),
            &type_operation_traits<util::array_element_t<T>>::get_operations()};
    }

    enum array_type array_type() const noexcept { return array_type_; }

    enum addressing_type addressing_type() const noexcept {
        return addressing_type_;
    }

    auto elem_type() const noexcept {
        return elem_type_;
    }

    any get(size_t idx, any&) const noexcept;
    any get_const(size_t idx, const any&) const noexcept;
    bool push_back(const any&, any&) const noexcept;
    bool pop_back(any&) const noexcept;
    any back(any&) const noexcept;
    any back_const(const any&) const noexcept;
    bool resize(size_t size, any& array) const noexcept;
    size_t size(const any&) const noexcept;
    size_t capacity(const any&) const noexcept;
    bool insert(size_t idx, const any&, any&) const noexcept;

private:
    enum array_type array_type_;
    enum addressing_type addressing_type_;
    const type* elem_type_;
    const array_operations* operations_;
    const type_operations* elem_operations_;

    array(const std::string& name, enum array_type arr_type,
          enum addressing_type addr_type, const type* elem_type,
          const array_operations* operations,
          const type_operations* elem_operations)
        : type{value_kind::Array, name},
          array_type_(arr_type),
          addressing_type_(addr_type),
          elem_type_(elem_type),
          operations_(operations),
          elem_operations_(elem_operations) {}

    template <typename T>
    static std::string get_name() {
        if constexpr (std::is_array_v<T>) {
            return "array";
        } else if constexpr (util::is_std_array_v<T>) {
            return "std::array";
        } else if constexpr (util::is_vector_v<T>) {
            return "std::vector";
        } else if constexpr (util::is_std_list_v<T>) {
            return "std::list";
        }

        return "unknown-array";
    }

    template <typename T>
    static enum array_type get_arr_type() {
        if constexpr (std::is_array_v<T> || util::is_std_array_v<T>) {
            return array_type::Static;
        } else {
            return array_type::Dynamic;
        }
    }

    template <typename T>
    static enum addressing_type get_addr_type() {
        if constexpr (std::is_array_v<T>) {
            return addressing_type::Random;
        } else {
            return std::is_same_v<typename std::iterator_traits<
                                      typename T::iterator>::iterator_category,
                                  std::random_access_iterator_tag>
                       ? addressing_type::Random
                       : addressing_type::Forward;
        }
    }
};

}  // namespace mirrow::drefl