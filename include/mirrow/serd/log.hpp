#pragma once

#include <iostream>

#ifndef MIRROW_LOG
#define MIRROW_LOG(expr) std::cout << (expr) << std::endl
#endif