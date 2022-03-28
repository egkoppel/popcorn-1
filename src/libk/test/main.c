#include <stdio.h>

#include "malloctest.h"
#include "malloctest2.h"

#define LOOP_SEED 1
#define RANDOM_SEED 11

int main() {
	printf("---- hugOS libk tester ----\n");
	
	test_malloc();
	
#if LOOP_SEED
	for (unsigned int seed = 0; seed < UINT32_MAX; ++seed) {
		printf("-------------------------- TRIALLING SEED: %u --------------------------\n", seed);
		test_malloc2(seed);
	}
#else
	test_malloc2(RANDOM_SEED);
#endif
	
	printf("---- concluded tests ----\n");
	
	return 0;
}
