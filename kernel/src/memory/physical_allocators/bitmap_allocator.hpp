/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_BITMAP_ALLOCATOR_HPP
#define HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_BITMAP_ALLOCATOR_HPP

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
	class BitmapAllocator : public IPhysicalAllocator {
	private:
		frame_t *start_frame;
		u64 *bitmap_start;
		u64 *bitmap_end;

		enum frame_state { free, allocated };

		int mark_frame(const frame_t *addr,
		               frame_state state = allocated) noexcept;      //!< Marks a frame with a given state
		frame_state get_frame(const frame_t *addr) const noexcept;   //!< Gets the state of a frame

		BitmapAllocator(paddr_t start_addr, u64 *bitmap_start, u64 *bitmap_end) noexcept :
			start_frame(aligned<paddr_t>::aligned_down(start_addr).frame()),
			bitmap_start(bitmap_start),
			bitmap_end(bitmap_end) {
			memset(bitmap_start, 0, (u8 *)bitmap_end - (u8 *)bitmap_start);
		}

		BitmapAllocator() noexcept : start_frame(nullptr), bitmap_start(nullptr), bitmap_end(nullptr) {}

	public:
		BitmapAllocator(BitmapAllocator&& rhs) noexcept            = default;
		BitmapAllocator& operator=(BitmapAllocator&& rhs) noexcept = default;

		frame_t *allocate_(u64 byte_length) override;
		//std::optional<memory::Frame> allocate_at(memory::Frame, uint64_t byte_length) override;
		void deallocate_(const frame_t *frames, u64) noexcept override;
		static BitmapAllocator
		from(paddr_t start_addr, MonotonicAllocator&&, u64 *bitmap_start, u64 *bitmap_end) noexcept;
	};
}   // namespace memory::physical_allocators

#endif   //HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_BITMAP_ALLOCATOR_HPP
