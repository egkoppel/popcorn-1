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
		auto [bitmap_index, bit_index] = this->frame_to_indices(addr);

		if (this->start_frame <= addr && bitmap_index < (this->bitmap.size() / 8)) {
			switch (state) {
				case allocated: this->bitmap[bitmap_index] &= ~(1ull << bit_index); break;
				case free: this->bitmap[bitmap_index] |= (1ull << bit_index); break;
			}
			return 0;
		}
		return -1;
	}

	template<class VAllocator>
	typename BitmapAllocator<VAllocator>::frame_state BitmapAllocator<VAllocator>::get_frame(const frame_t *addr)
			const noexcept {
		auto [bitmap_index, bit_index] = this->frame_to_indices(addr);

		if (this->start_frame <= addr && this->bitmap_start + bitmap_index < this->bitmap_end) {
			return this->bitmap_start[bitmap_index] & (1ull << bit_index) ? allocated : free;
		}
		return allocated; /* If bit outside of index then return allocated to imply unusable */
	}

	template<class VAllocator> frame_t *BitmapAllocator<VAllocator>::allocate_multi(u64 byte_length) {
		if (byte_length != constants::frame_size * 2) THROW(std::bad_alloc());

		for (usize i = 0; i < this->bitmap.size() / 8; i++) {
			decltype(auto) addr = this->bitmap[i];

			for (usize j = 0; j < 63; j++) {
				if ((addr & (3ull << j)) == (3ull << j)) {   // All bits have to be set to be able to allocate
					addr &= ~(3ull << j);                    // Clear bit to mark it as allocated
					u64 bits_to_start_of_iteration = i * 64;
					frame_t *ret_frame             = this->start_frame + (bits_to_start_of_iteration + j);
					return ret_frame;
				}
			}
		}
		THROW(std::bad_alloc());
	}

	template<class VAllocator> frame_t *BitmapAllocator<VAllocator>::allocate_(u64 byte_length) {
		/* TODO: Take byte length into account */
		if (byte_length > constants::frame_size) return this->allocate_multi(byte_length);

		for (usize i = 0; i < this->bitmap.size() / 8; i++) {
			decltype(auto) addr = this->bitmap[i];

			u64 first_set_bit = __builtin_ffsll(addr);
			if (first_set_bit != 0) {
				first_set_bit -= 1;                 // Since builtin offsets by 1
				addr &= ~(1ull << first_set_bit);   // Clear bit to mark it as allocated

				u64 bits_to_start_of_iteration = i * 64;
				frame_t *ret_frame             = this->start_frame + (bits_to_start_of_iteration + first_set_bit);
				return ret_frame;
			}
		}
		THROW(std::bad_alloc());
	}

	template<class VAllocator> void BitmapAllocator<VAllocator>::deallocate_(const frame_t *frames, u64 size) noexcept {
		LOG(Log::DEBUG, "BitmapAllocator deallocate %lp", frames->addr());
		auto frame_count = IDIV_ROUND_UP(size, constants::frame_size);
		for (auto f = frames; f < frames + frame_count; f++) {
			auto [bitmap_index, bit_index] = this->frame_to_indices(f);
			// assert(this->bitmap.start() + bitmap_index < this->bitmap.end() /*, "Attempted deallocation at %p
			//  outside of bitmap", addr*/);
			this->bitmap[bitmap_index] |= 1ull << bit_index;   // Set bit to mark it as deallocated
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
				for (auto i = aligned<paddr_t>::aligned_up(entry.get_start_address());
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
		for (auto i = monotonic_allocator.get_ramdisk_start_frame(); i < monotonic_allocator.get_ramdisk_end_frame();
		     i++) {
			allocator.mark_frame(i.frame(), allocated);
		}

		return allocator;
	}

	template<class VAllocator>
	std::tuple<u64, u64> BitmapAllocator<VAllocator>::frame_to_indices(const frame_t *frame) {
		uint64_t number_within_bitmap = frame->number() - this->start_frame->number();
		uint64_t bitmap_index         = number_within_bitmap / 64;
		uint64_t bit_index            = number_within_bitmap % 64;
		return std::make_tuple(bitmap_index, bit_index);
	}
}   // namespace memory::physical_allocators
