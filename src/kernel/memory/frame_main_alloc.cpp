/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "frame_main_alloc.hpp"
#include <stdio.h>
#include <panic.h>
#include <assert.h>

int FrameMainAllocator::set_bit(uint64_t addr) {
	uint64_t frame_num = (uint64_t)addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;

	fprintf(stdserial, "Set allocator bit for %p (%llx, %llx, %llx)\n", addr, frame_num, bitmap_index, bit_index);

	if (this->bitmap_start + bitmap_index < this->bitmap_end) {
		this->bitmap_start[bitmap_index] |= (1 << bit_index);
		return 0;
	}
	return -1;
}

int FrameMainAllocator::get_bit(uint64_t addr) {
	uint64_t frame_num = (uint64_t)addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;

	fprintf(stdserial, "Set allocator bit for %p (%llx, %llx, %llx)\n", addr, frame_num, bitmap_index, bit_index);

	if (this->bitmap_start + bitmap_index < this->bitmap_end) {
		this->bitmap_start[bitmap_index] |= (1 << bit_index);
		return (bool)(this->bitmap_start[bitmap_index] & (1 << bit_index));
	}
	return -1;
}

Option<uint64_t> FrameMainAllocator::allocate() {
	for (uint64_t *addr = this->bitmap_start; addr < this->bitmap_end; addr++) {
		if (*addr != UINT64_MAX) {
			uint64_t first_clear_bit = __builtin_ffsll(~(*addr));
			*addr |= (1ull << (first_clear_bit - 1));
			auto ret = ((uint64_t)addr - (uint64_t)this->bitmap_start + first_clear_bit - 1) * 0x1000;
			fprintf(stdserial, "Allocated frame %p\n", ret);
			return Some<uint64_t>(ret);
		}
	}
	return None<uint64_t>();
}

Option<uint64_t> FrameMainAllocator::allocate_at(uint64_t addr) {
	int allocated = this->get_bit(addr);
	if (!allocated) {
		this->set_bit(addr);
		return Some<uint64_t>(addr);
	} else return None<uint64_t>();
}

void FrameMainAllocator::deallocate(uint64_t addr) {
	fprintf(stdserial, "Deallocate frame at %p\n", addr);
	uint64_t frame_num = addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;
	assert_msg(this->bitmap_start + bitmap_index < this->bitmap_end, "Attempted deallocation at %p outside of bitmap", addr);
	this->bitmap_start[bitmap_index] &= ~(1ull << bit_index);
}

void FrameMainAllocator::init_from(FrameBumpAllocator& bump) {
	for (auto *i = reinterpret_cast<uint64_t *>(this->bitmap_start);
	     i < reinterpret_cast<uint64_t *>(this->bitmap_end); ++i) {
		*i = 0;
	}
	printf("init_alloc.next_alloc: %p\n", bump.get_next_alloc());
	for (uint64_t i = 0; i < bump.get_next_alloc(); i += 0x1000) {
		this->set_bit(i);
	}
	for (uint64_t i = bump.get_kernel_start(); i < bump.get_kernel_end(); i += 0x1000) {
		this->set_bit(i);
	}
	for (uint64_t i = bump.get_multiboot_start(); i < bump.get_multiboot_end(); i += 0x1000) {
		this->set_bit(i);
	}
	for (multiboot::memory_map_entry& entry : *bump.get_mem_map()) {
		if (entry.type != multiboot::memory_type::AVAILABLE) {
			for (uint64_t i = ALIGN_DOWN(entry.base_addr, 0x1000);
			     i < ALIGN_UP(entry.base_addr + entry.length, 0x1000); i += 0x1000) {
				this->set_bit(i);
			}
		}
	}
}
