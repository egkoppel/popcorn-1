#ifndef _HUGOS_IDT_H
#define _HUGOS_IDT_H

#include <stdint.h>
#include <assert.h>

namespace idt {
	struct __attribute__((packed)) entry {
		uint16_t pointer_low;
		uint16_t segment_selector;
		uint8_t ist;
		uint8_t type : 4;
		uint8_t _1 : 1;
		uint8_t dpl : 2;
		uint8_t present : 1;
		uint16_t pointer_middle;
		uint32_t pointer_high;
		uint32_t _2;

		entry();
	};

	struct __attribute__((packed)) idt_ptr {
		uint16_t size;
		uint64_t address;
	};

	template <uint8_t E> struct __attribute__((packed)) IDT {
		public:
		entry entries[E];

		IDT() {
			for (int i = 0; i < E; i++) {
				this->entries[i] = entry();
			}
		}

		void load() {
			idt_ptr ptr;
			ptr.size = sizeof(IDT<E>) - 1;
			ptr.address = reinterpret_cast<uint64_t>(this);

			__asm__ volatile("lidt %0" : : "m"(ptr));
		}

		void add_entry(uint8_t index, uint8_t dpl, void(*handler)(), uint8_t ist = 0) {
			assert_msg(E > index, "Index out of bounds");
			uint64_t ptr = (uint64_t)handler;

			this->entries[index].pointer_low = (uint16_t)ptr;
			this->entries[index].segment_selector = 0x8;
			this->entries[index].ist = ist;
			this->entries[index].type = 0xE;
			this->entries[index]._1 = 0;
			this->entries[index].dpl = dpl;
			this->entries[index].present = 1;
			this->entries[index].pointer_middle = (uint16_t)(ptr >> 16);
			this->entries[index].pointer_high = (uint32_t)(ptr >> 32);
			this->entries[index]._2 = 0;
		}
	};
}

#endif

