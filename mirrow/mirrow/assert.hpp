#pragma once

#include <cassert>
#include <iostream>

#ifndef MIRROW_ASSERT
#define MIRROW_ASSERT(x, expr) assert(((void)(expr), (x)))
#endif

#ifndef MIRROW_LOG
#define MIRROW_LOG(...) std::cout << __VA_ARGS__ << std::endl;
#endif