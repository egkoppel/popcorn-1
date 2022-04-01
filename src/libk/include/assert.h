#ifndef _HUGOS_ASSERT_H
#define _HUGOS_ASSERT_H

#include <panic.h>

#ifdef NDEBUG
#define assert(x) {}
#define assert_msg(x, s, ...) {}
#else
#define assert(x) {if (!(x)) panic("Assertion failed: " #x "\n");}
#define assert_msg(x, s, ...) {if (!(x)) panic("Assertion failed: " #x "\n" s, ##__VA_ARGS__);}
#endif

#ifndef __cplusplus
#define STATIC_ASSERT(cond)  typedef char __static_assert_type[ (cond) ? 1 : -1 ];
#endif

#endif
