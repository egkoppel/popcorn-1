/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_PHYSICAL_ALLOCATOR_HPP
#define HUGOS_PHYSICAL_ALLOCATOR_HPP

#include "physical_region.hpp"
#include "types.hpp"

#include <arch/constants.hpp>
#include <optional>
#include <stdint.h>
#include <utils.h>

namespace memory {
	struct frame_t;

	class IPhysicalAllocator {
	public:
		IPhysicalAllocator()                              = default;
		IPhysicalAllocator(const IPhysicalAllocator&)     = delete;
		IPhysicalAllocator(IPhysicalAllocator&&) noexcept = default;
		virtual ~IPhysicalAllocator()                     = default;

		IPhysicalAllocator& operator=(const IPhysicalAllocator&)     = delete;
		IPhysicalAllocator& operator=(IPhysicalAllocator&&) noexcept = default;

		/**
		 * Allocates at least enough frames to cover the requested size
		 * @param size Amount to allocate
		 * @return Reference into `mem_map` to the first allocated frame
		 * @throws std::bad_alloc Memory allocation error
		 * @note @p size should be determined deterministically as the same value will be used to deallocate
		 */
		frame_t *allocate(u64 size = memory::constants::frame_size);
		/**
		 *
		 * @param start Reference into `mem_map` to the frame returned by a call to allocate()
		 * @param size Size requested from allocation
		 */
		static void deallocate(const frame_t *start, u64 size = memory::constants::frame_size) noexcept;

	protected:
		/**
		 * other bob
		 * @return
		 */
		virtual frame_t *allocate_(u64) = 0;
		/**
		 * bob
		 */
		virtual void deallocate_(const frame_t *, u64) noexcept = 0;
	};

	/*class IPhysicalContiguousAllocator : public IPhysicalAllocator {
	public:
		virtual FrameRange allocate_at(Frame start, uint64_t byte_length) = 0;
		virtual FrameRange allocate_contiguous(uint64_t byte_length)      = 0;
	};*/
}   // namespace memory

#endif   //HUGOS_PHYSICAL_ALLOCATOR_HPP
