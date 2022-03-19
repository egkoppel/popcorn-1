#ifndef _HUGOS_MAIN_H
#define _HUGOS_MAIN_H

#include "../memory/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

extern allocator_vtable *global_frame_allocator;

#ifdef __cplusplus
}
#endif

#endif
