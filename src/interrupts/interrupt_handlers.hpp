#ifndef _HUGOS_INT_HANDLERS_H
#define _HUGOS_INT_HANDLERS_H

void init_idt();
extern "C" void syscall_long_mode_handler();

#endif
