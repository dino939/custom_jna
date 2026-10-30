//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_ASSERT_H
#define CUSTOM_JNA_ASSERT_H

#include <iostream>
#include <cstdlib>

#define assert(condition, message) \
do { \
if (!(condition)) { \
std::cerr << "Assertion failed: (" << #condition << "), " \
<< "message: " << message << ", " \
<< "file: " << __FILE__ << ", " \
<< "line: " << __LINE__ << std::endl; \
std::abort(); \
} \
} while (false)

#endif //CUSTOM_JNA_ASSERT_H