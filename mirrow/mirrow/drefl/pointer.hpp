#pragma once

#include "mirrow/drefl/value_kind.hpp"
#include "mirrow/util/misc.hpp"
#include "mirrow/drefl/type.hpp"


namespace mirrow::drefl {

struct type;

class pointer final: public type {
public:
    template <typename T>
    static pointer create(const type* pure_type) {
        static_assert(std::is_pointer_v<T>);
        std::string pointer_name = pure_type->name();
        int layer = util::pointer_layer_v<T>;
        for (int i = 0; i < layer; i++) {
            pointer_name += "*";
        }
        return {pointer_name,
                std::is_const_v<T>,
                std::is_const_v<util::remove_all_pointers_t<util::remove_cvref_t<T>>>,
                pure_type,
                layer};
    }

    bool is_const() const noexcept { return is_const_; }
    bool is_point_type_const() const noexcept { return is_point_type_const_; }

    const type* type_info() const noexcept { return typeinfo_; }

    int layers() const noexcept { return layers_; }

private:
    pointer(): type(value_kind::Pointer, "", nullptr) {}

    pointer(const std::string& name, bool is_const, bool is_point_type_const,
            const type* typeinfo, int layers)
        : type(value_kind::Pointer, name, nullptr),
          is_const_(is_const),
          is_point_type_const_(is_point_type_const),
          typeinfo_(typeinfo),
          layers_(layers) {}

    bool is_const_ = false;  // means `int* const`(not `const int*`)
    bool is_point_type_const_ = false;   // means `const int*`(not `int* const`)
    const type* typeinfo_;
    int layers_ = 0;  // pointer layer count: int** == 2, int* == 1
};

}  // namespace mirrow::drefl
