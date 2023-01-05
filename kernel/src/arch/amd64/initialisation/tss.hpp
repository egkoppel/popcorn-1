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
		uint32_t _res0;
		memory::vaddr_t privilege_stack_table[3];
		uint64_t _res1;
		memory::vaddr_t interrupt_stack_table[7];
		uint64_t _res2;
		uint16_t _res3;
		uint16_t io_map_base;

		TSS() noexcept;
		static void load(uint16_t gdt_index) noexcept;
		void add_stack(uint8_t stack_idx, const memory::KStack<>& stack) noexcept;
	};

	extern "C" TSS task_state_segment;
}   // namespace arch::amd64

#endif   // HUGOS_TSS_HPP
