/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_TSS_HPP
#define HUGOS_TSS_HPP

#include "memory/types.hpp"

#include <memory/stack.hpp>
#include <stdint.h>

namespace arch::amd64 {
	struct [[gnu::packed]] TSS {
		u32 _res0;
		memory::vaddr_t privilege_stack_table[3];
		u64 _res1;
		memory::vaddr_t interrupt_stack_table[7];
		u64 _res2;
		u16 _res3;
		u16 io_map_base;

		TSS() noexcept;
		static void load(u16 gdt_index) noexcept;
		void add_stack(u8 stack_idx, const memory::KStack<>& stack) noexcept;
	};

	extern "C" TSS task_state_segment;
}   // namespace arch::amd64

#endif   // HUGOS_TSS_HPP
