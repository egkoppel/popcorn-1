/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "debug.hpp"
#include <stdint.h>
#include <stdio.h>

/* Assume, as is often the case, that RBP is the first thing pushed. If not, we are in trouble. */
struct stackframe {
	struct stackframe* rbp;
	uint64_t rip;
};

void trace_stack_trace(unsigned int MaxFrames, uint64_t rbp) noexcept {
	auto *stk = reinterpret_cast<struct stackframe*>(rbp);
	fprintf(stdserial, "Stack trace:\n");
	for(unsigned int frame = 0; stk && stk->rbp && frame < MaxFrames; ++frame)
	{
		// Unwind to previous stack frame
		fprintf(stdserial, "\t0x%llx\n", stk->rip);
		stk = stk->rbp;
	}
}