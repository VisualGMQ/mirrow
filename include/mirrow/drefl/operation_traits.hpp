#pragma once

#include "mirrow/util/variable_traits.hpp"
#include "mirrow/drefl/exception.hpp"
#include "mirrow/assert.hpp"
#include <tuple>
#include <type_traits>

namespace mirrow::drefl {

struct type_operations final {
    using destroy_fn = void(void*);
    using copy_construct_fn = void*(void*);
    using steal_construct_fn = void*(void*);
    using copy_assignment_fn = void(void*, void*);
    using steal_assignment_fn = void(void*, void*);

    destroy_fn* destroy = empty_destroy;
    copy_construct_fn* copy_construct = empty_copy;
    steal_construct_fn* steal_construct = empty_steal;
    copy_assignment_fn* copy_assignment = empty_copy_assign;
    steal_assignment_fn* steal_assignment = empty_steal_assign;

    static type_operations null;

private:
    static void empty_destroy(void*) {}
    static void* empty_copy(void*) { return nullptr; }
    static void* empty_steal(void*) { return nullptr; }
    static void empty_copy_assign(void*, void*) {}
    static void empty_steal_assign(void*, void*) {}
};

template <typename T>
struct type_operation_traits {
    static void destroy(void* elem) {
        if constexpr (std::is_destructible_v<T>) {
            if constexpr (std::is_array_v<T>){
                delete[] (T*)(elem);
            } else {
                delete (T*)(elem);
            }
        } else {
            MIRROW_LOG("type don't support destruct");
        }
    }

    static void* copy_construct(void* elem) {
        if constexpr (std::is_copy_constructible_v<T>) {
            return new T{*(const T*)elem};
        } else {
            MIRROW_LOG("type don't support copy construct");
            return nullptr;
        }
    }

    static void* steal_construct(void* elem) {
        if constexpr (std::is_move_constructible_v<T>) {
            return new T{std::move(*(T*)elem)};
        } else {
            MIRROW_LOG("type don't support move construct");
            return nullptr;
        }
    }

    static void copy_assignment(void* dst, void* src) {
        if constexpr (std::is_copy_assignable_v<T>) {
            *(T*)(dst) = *(const T*)(src);
        } else {
            MIRROW_LOG("type don't support copy assignment");
        }
    }

    static void steal_assignment(void* dst, void* src) {
        if constexpr (std::is_copy_assignable_v<T>) {
            *(T*)(dst) = std::move(*(T*)(src));
        } else {
            MIRROW_LOG("type don't support copy assignment");
        }
    }

    static auto& get_operations() {
        using traits = type_operation_traits<T>;

        static type_operations operations = {
            traits::destroy, traits::copy_construct, traits::steal_construct,
            traits::copy_assignment, traits::steal_assignment};
        return operations;
    }
};

}  // namespace mirrow::drefl