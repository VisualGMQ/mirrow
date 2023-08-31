/**
 * @file srefl_macro.hpp
 * @brief some helpful macros to help you reflect class more eazier
 * @note use `srefl_end.hpp` when you finish reflect
 */

#pragma once
#include "mirrow/srefl/srefl.hpp"

#define srefl_class(type) template<> struct type_info<type>: base_type_info<type>

#define fields using fields = util::type_list

#define field field_traits

#define bases using bases = util::type_list

#define ctors using ctors = util::type_list

#ifdef MIRROW_SREFL_BEGIN
#error "do you forget include mirrow/srefl/srefl_end.hpp after include mirrow/srefl/srefl_begin.hpp?"
#define MIRROW_SREFL_BEGIN
#endif

namespace mirrow::srefl {
