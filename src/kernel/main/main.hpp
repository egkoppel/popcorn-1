#ifndef _HUGOS_MAIN_H
#define _HUGOS_MAIN_H

#include "../memory/allocator.h"
#include "../gdt/gdt.hpp"
#include "../gdt/tss.hpp"

extern "C" {

extern allocator_vtable *global_frame_allocator;
extern gdt::GDT global_descriptor_table;
extern tss::TSS task_state_segment;

}

#endif
