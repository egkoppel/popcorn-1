/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_IDT_HPP
#define HUGOS_IDT_HPP

#include "assert.h"
#include "memory/types.hpp"

#include <cstdint>

namespace arch::amd64 {
	template<uint16_t E> requires(E <= 256)
	class [[gnu::packed]] IDT {
	public:
		class [[gnu::packed]] alignas(8) Entry {
			friend class IDT;
		private:
			uint16_t pointer_low      = 0;
			uint16_t segment_selector = 0;
			uint8_t ist               = 0;
			uint8_t type    : 4       = 0;
			uint8_t _1      : 1       = 0;
			uint8_t dpl     : 2       = 0;
			uint8_t present : 1       = 0;
			uint16_t pointer_middle   = 0;
			uint32_t pointer_high     = 0;
			uint32_t _2               = 0;

		public:
			constexpr Entry() noexcept = default;
			constexpr Entry(memory::vaddr_t handler, uint8_t dpl, uint8_t ist_idx) noexcept {
				this->pointer_low      = (uint16_t)handler.address;
				this->segment_selector = 0x8;
				this->ist              = ist_idx;
				this->type             = 0xE;
				this->_1               = 0;
				this->dpl              = dpl;
				this->present          = 1;
				this->pointer_middle   = (uint16_t)(handler.address >> 16);
				this->pointer_high     = (uint32_t)(handler.address >> 32);
				this->_2               = 0;
			}
		};

	private:
		struct [[gnu::packed]] idt_ptr_t {
			uint16_t size;
			uint64_t address;
		};

		Entry entries[E];

	public:
		constexpr IDT() noexcept = default;

		void load() noexcept {
			idt_ptr_t ptr{.size = sizeof(IDT<E>) - 1, .address = reinterpret_cast<uint64_t>(this)};
			__asm__ volatile("lidt %0" : : "m"(ptr));
		}

		void add_entry(uint8_t index, uint8_t dpl, void (*handler)() noexcept, uint8_t ist = 0) noexcept {
			assert(E > index);
			this->entries[index] = Entry{memory::vaddr_t{.address = reinterpret_cast<usize>(handler)}, dpl, ist};
		}

		constexpr void set_flags(uint8_t index, uint8_t dpl, uint8_t ist) {
			this->entries[index].dpl = dpl;
			this->entries[index].ist = ist;
		}
	};

	extern IDT<256> interrupt_descriptor_table;
}   // namespace arch::amd64

#endif   // HUGOS_IDT_HPP
