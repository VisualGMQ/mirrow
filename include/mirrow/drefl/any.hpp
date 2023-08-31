#pragma once

#include <type_traits>

namespace mirrow {

namespace drefl {

class any final {
public:
    using storage_type = void*;

    template <typename T>
    any() {};

private:
    template <typename T>
    struct traits {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;

        template <typename... Args>
        static void* create(storage_type& storage, Args&&... args) {
            auto instance = std::make_unique<type>(std::forward<Args>(args)...);
            new (&storage) type*{instance.get()};
            return instance.release();
        }

        static void copy(storage_type& storage, cosnt void* other){
            auto instance = std::make_unique<type>(*static_cast<const type*>(other));
            new (&storage) type*{instance.get()};
            return instance.release();
        };

        static destroy(void* obj) {
            auto actual = static_cast<type*>(obj);
            delete actual;
        }

        static steal(storage_type& to, const void* from) {
            auto actual = static_cast<const type*>(from);
            new (&to) type*(actual);
        }
    };


    void* instance_ = nullptr;
};

}

}