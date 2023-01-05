/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_BITMAP_ALLOCATOR_HPP
#define HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_BITMAP_ALLOCATOR_HPP

#include "../memory_map.hpp"
#include "../physical_allocator.hpp"
#include "../types.hpp"
#include "monotonic_allocator.hpp"

#include <multiboot/multiboot.hpp>

namespace memory::physical_allocators {
	/**
	 * @brief Add stuff
	 *
	 * Stores internal state as `1` meaning free and `0` meaning allocated
	 */
	template<class VAllocator> class BitmapAllocator : public IPhysicalAllocator {
	private:
		frame_t *start_frame;
		MemoryMap<u64, VAllocator> bitmap;

		enum frame_state { free, allocated };

		static constexpr auto flags =
				paging::PageTableFlags::WRITEABLE | paging::PageTableFlags::NO_EXECUTE | paging::PageTableFlags::GLOBAL;

		int mark_frame(const frame_t *addr,
		               frame_state state = allocated) noexcept;      //!< Marks a frame with a given state
		frame_state get_frame(const frame_t *addr) const noexcept;   //!< Gets the state of a frame

		BitmapAllocator(paddr_t start_addr,
		                std::size_t size,
		                IPhysicalAllocator& page_allocator,
		                VAllocator allocator) noexcept :
			start_frame(aligned<paddr_t>::aligned_down(start_addr).frame()),
			bitmap(size, flags, page_allocator, paging::kas, allocator) {
			memset(this->bitmap.operator->(), 0, size);
		}

		BitmapAllocator() noexcept : start_frame(nullptr), bitmap() {}

	public:
		BitmapAllocator(BitmapAllocator&& rhs) noexcept            = default;
		BitmapAllocator& operator=(BitmapAllocator&& rhs) noexcept = default;

		frame_t *allocate_(u64 byte_length) override;
		//std::optional<memory::Frame> allocate_at(memory::Frame, uint64_t byte_length) override;
		void deallocate_(const frame_t *frames, u64) noexcept override;
		static BitmapAllocator from(paddr_t start_addr, std::size_t size, MonotonicAllocator&&, VAllocator allocator);
	};
}   // namespace memory::physical_allocators

#include "bitmap_allocator.ipp"

#endif   //HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_BITMAP_ALLOCATOR_HPP
