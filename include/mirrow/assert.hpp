#pragma once

#include <cassert>

#ifndef MIRROW_ASSERT
#define MIRROW_ASSERT(x, expr) assert(((void)expr, x))
#endif