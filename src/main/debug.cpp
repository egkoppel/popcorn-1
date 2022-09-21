#include "debug.hpp"
#include <stdint.h>
#include <stdio.h>

/* Assume, as is often the case, that RBP is the first thing pushed. If not, we are in trouble. */
struct stackframe {
	struct stackframe* rbp;
	uint64_t rip;
};

void trace_stack_trace(unsigned int MaxFrames, uint64_t rbp) {
	auto *stk = reinterpret_cast<struct stackframe*>(rbp);
	fprintf(stdserial, "Stack trace:\n");
	for(unsigned int frame = 0; stk && stk->rbp && frame < MaxFrames; ++frame)
	{
		// Unwind to previous stack frame
		fprintf(stdserial, "\t0x%llx\n", stk->rip);
		stk = stk->rbp;
	}
}