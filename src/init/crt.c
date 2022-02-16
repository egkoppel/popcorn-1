#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

#include <termcolor.h>

#define ATEXIT_COUNT 128
#define NULL ((void*)0)

typedef struct {
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
} atexit_func_entry_t;

atexit_func_entry_t atexit_funcs[ATEXIT_COUNT] = {0};
unsigned int atexit_used = 0;

void *__dso_handle = 0;

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso) {
	if (atexit_used >= ATEXIT_COUNT) return -1;
	atexit_funcs[atexit_used++] = (atexit_func_entry_t){
		.destructor_func = f,
		.obj_ptr = objptr,
		.dso_handle = dso
	};
	return 0;
}

void __cxa_finalize(void *dso) {
	for (int i = ATEXIT_COUNT - 1; i >= 0; --i) {
		if (dso == NULL || atexit_funcs[i].dso_handle == dso) {
			if (atexit_funcs[i].destructor_func != NULL) atexit_funcs[i].destructor_func(atexit_funcs[i].obj_ptr);
			atexit_funcs[i].destructor_func = NULL;
		}
	}
}

void __cxa_pure_virtual() {

}

typedef void(*ctor_func)(void);

extern ctor_func start_ctors;
extern ctor_func end_ctors;
extern ctor_func init_array_start;
extern ctor_func init_array_end;

void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr);

void __cxa_init(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	kprintf("[    ] Running ctors\n\tstart: %p\n\t  end: %p\n", &start_ctors, &end_ctors);
	ctor_func *i = &start_ctors;
	while (i < &end_ctors) {
		//kprintf("Calling constructor at %p\n", i);
		if (*i != NULL) (*i)();
		i++;
	}
	kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Ran ctors\n");

	kprintf("[    ] Running elements of init array\n\tstart: %p\n\t  end: %p\n", &init_array_start, &init_array_end);
	i = &init_array_start;
	while (i < &init_array_end) {
		//kprintf("Calling constructor at 0x%x\n", i);
		if (*i != NULL) (*i)();
		i++;
	}
	kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Done running elements of init array\n");

	kmain(multiboot_magic, multiboot_addr);

	__cxa_finalize(NULL);

	while(1);
}