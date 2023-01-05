/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bitmap_allocator.hpp"

#include <assert.h>
#include <cstdio>
#include <limits>
#include <panic.h>

namespace memory::physical_allocators {
	template<class VAllocator>
	int BitmapAllocator<VAllocator>::mark_frame(const frame_t *addr, frame_state state) noexcept {
		uint64_t frame_num    = addr->number() - this->start_frame->number();
		uint64_t bitmap_index = frame_num / 64;
		uint64_t bit_index    = frame_num % 64;

		if (this->start_frame <= addr && bitmap_index < this->bitmap.size()) {
			switch (state) {
				case allocated: this->bitmap[bitmap_index] &= ~(1ull << bit_index); break;
				case free: this->bitmap[bitmap_index] |= (1ull << bit_index); break;
			}
			return 0;
		}
		return -1;
	}

	template<class VAllocator>
	typename BitmapAllocator<VAllocator>::frame_state
	BitmapAllocator<VAllocator>::get_frame(const frame_t *addr) const noexcept {
		uint64_t frame_num    = addr->number() - this->start_frame->number();
		uint64_t bitmap_index = frame_num / 64;
		uint64_t bit_index    = frame_num % 64;

		if (this->start_frame <= addr && this->bitmap_start + bitmap_index < this->bitmap_end) {
			return this->bitmap_start[bitmap_index] & (1ull << bit_index) ? allocated : free;
		}
		return allocated; /* If bit outside of index then return allocated to imply unusable */
	}

	template<class VAllocator> frame_t *BitmapAllocator<VAllocator>::allocate_(u64 byte_length) {
		/* TODO: Take byte length into account */
		if (byte_length > constants::frame_size) throw std::bad_alloc();

		for (usize i = 0; i < this->bitmap.size(); i++) {
			decltype(auto) addr = this->bitmap[i];

			u64 first_set_bit = __builtin_ffsll(addr);
			if (first_set_bit != 0) {
				addr &= ~(1ull << (first_set_bit - 1));   // Clear bit to mark it as allocated

				u64 bits_to_start_of_iteration = i * 8;
				frame_t *ret_frame             = this->start_frame + (bits_to_start_of_iteration + first_set_bit - 1);
				return ret_frame;
			}
		}
		throw std::bad_alloc();
	}

	/*std::optional<Frame> PhysicalBitmapAllocator::allocate_at(Frame addr, uint64_t byte_length) {
	TODO: Take byte length into account
	int allocated = this->get_bit(addr);
	if (!allocated) {
		if (this->set_bit(addr)) return std::nullopt;
		return {addr};
	} else return std::nullopt;
}*/

	template<class VAllocator> void BitmapAllocator<VAllocator>::deallocate_(const frame_t *frames, u64 size) noexcept {
		auto frame_count = IDIV_ROUND_UP(size, constants::frame_size);
		for (auto f = frames; f < frames + frame_count; f++) {
			uint64_t frame_num    = f->number();
			uint64_t bitmap_index = frame_num / 64;
			uint64_t bit_index    = frame_num % 64;
			//FIXME: assert(this->bitmap_start + bitmap_index < this->bitmap_end /*, "Attempted deallocation at %p outside of bitmap", addr*/);
			this->bitmap[bitmap_index] |= 1ull << bit_index;   // Set bit to mark it as allocated
		}
	}

	template<class VAllocator>
	BitmapAllocator<VAllocator> BitmapAllocator<VAllocator>::from(paddr_t start_addr,
	                                                              std::size_t size,
	                                                              MonotonicAllocator&& monotonic_allocator,
	                                                              VAllocator virtual_allocator) {
		BitmapAllocator<VAllocator> allocator{start_addr, size, monotonic_allocator, virtual_allocator};

		for (auto& entry : *monotonic_allocator.get_mem_map()) {
			if (entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE) {
				for (auto i = aligned<paddr_t>::aligned_down(entry.get_start_address());
				     i <= aligned<paddr_t>::aligned_down(entry.get_end_address());
				     i++) {
					allocator.mark_frame(i.frame(), free);
				}
			}
		}

		for (auto i = 0_palign; i < monotonic_allocator.get_next_frame(); i++) {
			allocator.mark_frame(i.frame(), allocated);
		}
		for (auto i = monotonic_allocator.get_kernel_start_frame(); i < monotonic_allocator.get_kernel_end_frame();
		     i++) {
			allocator.mark_frame(i.frame(), allocated);
		}
		for (auto i = monotonic_allocator.get_multiboot_start_frame();
		     i < monotonic_allocator.get_multiboot_end_frame();
		     i++) {
			allocator.mark_frame(i.frame(), allocated);
		}

		return allocator;
	}

}   // namespace memory::physical_allocators
