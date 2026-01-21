/**
 * @file srefl_macro.hpp
 * @brief some helpful macros to help you reflect class more eazier
 * @note use `srefl_end.hpp` when you finish reflect
 */

#include "mirrow/srefl/reflect.hpp"
#include <array>

#define srefl_class(type, ...)                      \
    template <>                                     \
    struct type_info<type> : base_type_info<type> { \
        static constexpr std::string_view name() {  \
            return strip_name(#type);               \
        }                                           \
        __VA_ARGS__                                 \
    };

#define fields(...) \
    inline static constexpr auto fields = std::make_tuple(__VA_ARGS__);

#define field(pointer, ...)              \
    field_traits {                       \
        pointer, #pointer, ##__VA_ARGS__ \
    }

#define bases(...) using bases = util::type_list<__VA_ARGS__>;

#define ctors(...) using ctors = util::type_list<__VA_ARGS__>;

#define ctor(...) ctor<__VA_ARGS__>

#define srefl_enum(type, ...)                               \
    template <>                                             \
    struct type_info<type> : base_type_info<type> {         \
        static constexpr std::string_view name() {          \
            return #type;                                   \
        }                                                   \
        static constexpr std::array enums = {__VA_ARGS__}; \
    };

#define enum_value(value, name) \
    enum_value {                \
        value, name             \
    }

#ifdef MIRROW_SREFL_BEGIN
#error \
    "do you forget include mirrow/srefl/srefl_end.hpp after include mirrow/srefl/srefl_begin.hpp?"
#define MIRROW_SREFL_BEGIN
#endif

namespace mirrow::srefl {
