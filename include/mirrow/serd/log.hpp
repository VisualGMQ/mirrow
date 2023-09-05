#pragma once

#include <iostream>

#ifndef LOG
#define LOG(expr) std::cout << (expr) << std::endl
#endif