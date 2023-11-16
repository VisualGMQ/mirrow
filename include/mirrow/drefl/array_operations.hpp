#pragma once

#include <array>
#include <list>
#include <vector>

namespace mirrow::drefl {

struct array_operations final {
    using get_fn = void*(size_t, void*, bool);
    using push_back_fn = bool(const void*, void*);
    using pop_back_fn = bool(void*);
    using back_fn = void*(void*, bool);
    using resize_fn = bool(size_t, void*);
    using size_fn = size_t(const void*);
    using capacity_fn = size_t(const void*);
    using insert_fn = bool(size_t, const void*, void*);

    static array_operations null;

    get_fn* get = empty_get;
    push_back_fn* push_back = empty_push_back;
    back_fn* back = empty_back;
    pop_back_fn* pop_back = empty_pop_back;
    resize_fn* resize = empty_resize;
    size_fn* size = empty_size;
    capacity_fn* capacity = empty_capacity;
    insert_fn* insert = empty_insert;

private:
    static void* empty_get(size_t, void*, bool) { return nullptr; }

    static bool empty_push_back(const void*, void*) { return false; }

    static bool empty_pop_back(void*) { return false; }

    static void* empty_back(void*, bool) { return nullptr; }

    static bool empty_resize(size_t, void*) { return false; }

    static size_t empty_size(const void*) { return 0; }

    static size_t empty_capacity(const void*) { return 0; }

    static bool empty_insert(size_t, const void*, void*) { return false; }
};

template <typename T>
struct array_operation_traits;

// sperialize for T[N]
template <typename T, size_t N>
struct array_operation_traits<T[N]> final {
    using type = T[N];

    static void* get(size_t idx, void* array, bool is_const) {
        if (is_const) {
            return (void*)((const T*)array + idx);
        } else {
            return (T*)array + idx;
        }
    }

    static bool push_back(const void*, void*) { return false; }

    static bool pop_back(void*) { return false; }

    static void* back(void*, bool) { return nullptr; }

    static bool resize(size_t, void*) { return false; }

    static size_t size(const void*) { return N; }

    static size_t capacity(const void*) { return N; }

    static bool insert(size_t, const void*, void*) { return false; }

    static auto& get_operations() {
        using traits = array_operation_traits<type>;

        static array_operations operations = {
            traits::get,      traits::push_back, traits::back,
            traits::pop_back, traits::resize,    traits::size,
            traits::capacity, traits::insert};
        return operations;
    }
};

// sperialize for std::array
template <typename T, size_t N>
struct array_operation_traits<std::array<T, N>> final {
    using type = std::array<T, N>;

    static void* get(size_t idx, void* array, bool is_const) {
        if (is_const) {
            return (void*)&((const type*)array)->operator[](idx);
        } else {
            return &((type*)array)->operator[](idx);
        }
    }

    static bool push_back(const void*, void*) { return false; }

    static bool pop_back(void*) { return false; }

    static void* back(void*, bool) { return nullptr; }

    static bool resize(size_t, void*) { return false; }

    static size_t size(const void*) { return N; }

    static size_t capacity(const void*) { return N; }

    static bool insert(size_t, const void*, void*) { return false; }

    static auto& get_operations() {
        using traits = array_operation_traits<type>;

        static array_operations operations = {
            traits::get,      traits::push_back, traits::back,
            traits::pop_back, traits::resize,    traits::size,
            traits::capacity, traits::insert};
        return operations;
    }
};

// specialize for std::vector
template <typename T, typename Alloc>
struct array_operation_traits<std::vector<T, Alloc>> final {
    using type = std::vector<T, Alloc>;

    static void* get(size_t idx, void* array, bool is_const) {
        if (is_const) {
            return (void*)&((const type*)array)->operator[](idx);
        } else {
            return &((type*)array)->operator[](idx);
        }
    }

    static bool push_back(const void* elem, void* array) {
        ((type*)array)->push_back(*((T*)elem));
        return true;
    }

    static bool pop_back(void* array) {
        ((type*)array)->pop_back();
        return true;
    }

    static void* back(void* array, bool is_const) {
        if (is_const) {
            return (void*)&((const type*)array)->back();
        } else {
            return &((type*)array)->back();
        }
    }

    static bool resize(size_t idx, void* array) {
        ((type*)array)->resize(idx);
        return true;
    }

    static size_t size(const void* array) { return ((type*)array)->size(); }

    static size_t capacity(const void* array) {
        return ((type*)array)->capacity();
    }

    static bool insert(size_t idx, const void* elem, void* array) {
        type* arr = (type*)array;
        auto it = arr->insert(arr->begin() + idx, *((T*)elem));
        return it != arr->end();
    }

    static auto& get_operations() {
        using traits = array_operation_traits<type>;

        static array_operations operations = {
            traits::get,      traits::push_back, traits::back,
            traits::pop_back, traits::resize,    traits::size,
            traits::capacity, traits::insert};
        return operations;
    }
};

// specialize for std::list
template <typename T, typename Alloc>
struct array_operation_traits<std::list<T, Alloc>> final {
    using type = std::list<T, Alloc>;

    static void* get(size_t idx, void* array, bool is_const) {
        if (is_const) {
            auto arr = (const type*)array;
            auto it = arr->begin();
            while (idx > 0) {
                it++;
                idx--;
            }
            return (void*)&(*it);
        } else {
            auto arr = (type*)array;
            auto it = arr->begin();
            while (idx > 0) {
                it++;
                idx--;
            }
            return &(*it);
        }
    }

    static bool push_back(const void* elem, void* array) {
        ((type*)array)->push_back(*((T*)elem));
        return true;
    }

    static bool pop_back(void* array) {
        ((type*)array)->pop_back();
        return true;
    }

    static void* back(void* array, bool is_const) {
        if (is_const) {
            return (void*)&((const type*)array)->back();
        } else {
            return &((type*)array)->back();
        }
    }

    static bool resize(size_t idx, void* array) {
        ((type*)array)->resize(idx);
        return true;
    }

    static size_t size(const void* array) { return ((type*)array)->size(); }

    static size_t capacity(const void* array) { return ((type*)array)->size(); }

    static bool insert(size_t idx, const void* elem, void* array) {
        type* arr = (type*)array;
        auto it = arr->begin();
        while (idx > 0) {
            idx--;
            it++;
        }
        auto result = arr->insert(it, *((T*)elem));
        return result != arr->end();
    }

    static auto& get_operations() {
        using traits = array_operation_traits<type>;

        static array_operations operations = {
            traits::get,      traits::push_back, traits::back,
            traits::pop_back, traits::resize,    traits::size,
            traits::capacity, traits::insert};
        return operations;
    }
};

}  // namespace mirrow::drefl