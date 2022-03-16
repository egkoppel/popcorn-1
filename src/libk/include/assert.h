#ifndef _HUGOS_ASSERT_H
#define _HUGOS_ASSERT_H

#include <panic.h>

#define assert(x, s, ...) {if (!(x)) panic("Assertion failed: " #x "\n" s, ##__VA_ARGS__);}

#endif
