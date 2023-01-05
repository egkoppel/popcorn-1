/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "malloctest.h"
#include "malloctest2.h"
#include "malloctest3.h"

#include <stdio.h>

#define ENABLE_MALLOCTEST  1
#define ENABLE_MALLOCTEST2 1
#define ENABLE_MALLOCTEST3 1

#define LOOP_SEED          1

#if LOOP_SEED
	#define LOOP_SEED_START 0
	#define LOOP_SEED_END   100   //UINT32_MAX
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
		printf("\r-------------------------- malloctest2: TRIALLING SEED: %9u --------------------------", seed);
		fflush(stdout);
		test_malloc2(seed);
	}
	printf("\n");
	#else
	test_malloc2(RANDOM_SEED);
	#endif
#else
	printf("[ Not running test_malloc2() since it is disabled ]\n");
#endif

#if ENABLE_MALLOCTEST3
	#if LOOP_SEED
	for (unsigned int seed = LOOP_SEED_START; seed < LOOP_SEED_END; ++seed) {
		printf("\r-------------------------- malloctest3: TRIALLING SEED: %9u --------------------------", seed);
		fflush(stdout);
		test_malloc3(seed);
	}
	printf("\n");
	#else
	test_malloc3(RANDOM_SEED);
	#endif
#else
	printf("[ Not running test_malloc3() since it is disabled ]\n");
#endif

	printf("---- concluded tests ----\n");

	return 0;
}
