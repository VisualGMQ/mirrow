/**
 * @file srefl_macro.hpp
 * @brief some helpful macros to help you reflect class more eazier
 * @note use `srefl_end.hpp` when you finish reflect
 */

#pragma once
#include "mirrow/srefl/srefl.hpp"

#define srefl_class(type) template<> struct ::mirrow::srefl::type_info<type>: ::mirrow::srefl::base_type_info<type>

#define fields using fields = ::mirrow::util::type_list

#define field ::mirrow::srefl::field_traits

#define bases using bases = typename ::mirrow::util::type_list

#define ctors using ctors = ::mirrow::util::type_list

#define ctor ::mirrow::srefl::ctor
