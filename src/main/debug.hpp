#ifndef _HUGOS_DEBUG_H
#define _HUGOS_DEBUG_H

#include <stdint.h>

void trace_stack_trace(unsigned int MaxFrames, uint64_t rbp);

#endif