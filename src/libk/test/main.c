#include <stdio.h>

#include "malloctest.h"
#include "malloctest2.h"

#define ENABLE_MALLOCTEST  0
#define ENABLE_MALLOCTEST2 1

#define LOOP_SEED 1

#if LOOP_SEED
#define LOOP_SEED_START 0
#define LOOP_SEED_END UINT32_MAX
#else
#define RANDOM_SEED 11
#endif

int main() {
	printf("---- hugOS libk tester ----\n");
	
#if ENABLE_MALLOCTEST
	test_malloc();
#else
	printf("[ Not running test_malloc() since it is disabled ]\n");
#endif
	
#if ENABLE_MALLOCTEST2
#if LOOP_SEED
	for (unsigned int seed = LOOP_SEED_START; seed < LOOP_SEED_END; ++seed) {
		printf("\r-------------------------- TRIALLING SEED: %9u --------------------------", seed);
		test_malloc2(seed);
	}
	printf("\n");
#else
	test_malloc2(RANDOM_SEED);
#endif
#else
	printf("[ Not running test_malloc2() since it is disabled ]\n");
#endif
	
	printf("---- concluded tests ----\n");
	
	return 0;
}
