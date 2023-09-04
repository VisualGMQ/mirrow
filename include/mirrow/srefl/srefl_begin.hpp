/**
 * @file srefl_macro.hpp
 * @brief some helpful macros to help you reflect class more eazier
 * @note use `srefl_end.hpp` when you finish reflect
 */

#pragma once
#include "mirrow/srefl/srefl.hpp"

#define srefl_class(type) template<> struct type_info<type>: base_type_info<type>

#define fields(...) inline static constexpr auto fields = std::make_tuple(__VA_ARGS__);

#define field(pointer, ...) field_traits{pointer, #pointer, ##__VA_ARGS__}

#define bases(...) using bases = util::type_list<__VA_ARGS__>;

#define ctors(...) using ctors = util::type_list<__VA_ARGS__>;

#define ctor(...) ctor<__VA_ARGS__>

#ifdef MIRROW_SREFL_BEGIN
#error "do you forget include mirrow/srefl/srefl_end.hpp after include mirrow/srefl/srefl_begin.hpp?"
#define MIRROW_SREFL_BEGIN
#endif

namespace mirrow::srefl {
