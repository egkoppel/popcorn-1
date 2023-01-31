/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
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
#include <popcorn_prelude.h>
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
		 * Allocates at least enough frames to cover the requested size starting at the requested address
		 * @param at The address to start at
		 * @param size Amount to allocate
		 * @return Reference into `mem_map` to the first allocated frame
		 * @throws std::bad_alloc Memory allocation error
		 * @note @p size should be determined deterministically as the same value will be used to deallocate
		 */
		frame_t *allocate(aligned<paddr_t> at, u64 size = memory::constants::frame_size);

		/**
		 *
		 * @param start Reference into `mem_map` to the frame returned by a call to allocate()
		 * @param size Size requested from allocation
		 */
		static void drop(frame_t *start, u64 size = memory::constants::frame_size) noexcept;

	protected:
		/**
		 * other bob
		 * @return
		 */
		virtual frame_t *allocate_(u64) = 0;
		/**
		 * other bob 2
		 * @return
		 */
		virtual frame_t *allocate_at_(aligned<paddr_t>, u64) { THROW(std::bad_alloc()); }
		/**
		 * bob
		 */
		virtual void deallocate_(const frame_t *, u64) noexcept = 0;

		virtual const char *name() noexcept = 0;
	};

}   // namespace memory

#endif   // HUGOS_PHYSICAL_ALLOCATOR_HPP
