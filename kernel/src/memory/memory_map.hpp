/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MEMORY_MAPPER_HPP
#define HUGOS_MEMORY_MAPPER_HPP

#include "paging.hpp"
#include "physical_region.hpp"
#include "virtual_allocator.hpp"
#include "virtual_region.hpp"

#include <optional>
#include <threading/scheduler.hpp>
#include <utils.h>

namespace memory {
	/**
	 * @brief A mapping between virtual and physical memory
	 * @tparam VAllocator The virtual allocator to use
	 * @tparam T The type of the underlying object
	 *
	 * A MemoryMap represents a RAII-based map between virtual and physical memory space, and should usually be treated similarly to a pointer by specifying the type of \p T.
	 */
	template<class T = void, class VAllocator = general_allocator_t> class MemoryMap {
	public:
		/**
		 * Creates an anonymous memory mapping
		 * @param byte_count
		 * @param flags
		 * @param page_allocator
		 * @param in
		 */
		MemoryMap(usize byte_count,
		          paging::PageTableFlags flags,
		          IPhysicalAllocator& page_allocator,
		          paging::AddressSpace& in = threads::local_scheduler->get_current_task()->address_space());

		/**
		 * Creates a memory mapping at the requested location
		 * @param at
		 * @param byte_count
		 * @param flags
		 * @param page_allocator
		 * @param in
		 */
		MemoryMap(paddr_t at,
		          usize byte_count,
		          paging::PageTableFlags flags,
		          IPhysicalAllocator& page_allocator,
		          paging::AddressSpace& in = threads::local_scheduler->get_current_task()->address_space());
		MemoryMap(const MemoryMap&) = delete;
		~MemoryMap();

		MemoryMap& operator=(MemoryMap&&);

		template<class _u = T>
		_u& operator*()
			requires(!std::is_void_v<_u>);
		template<class _u = T>
		_u *operator->()
			requires(!std::is_void_v<_u>);

		template<class U>
		explicit operator MemoryMap<U, VAllocator>()
			requires(std::is_convertible_v<T *, U *>);

		void expand(usize new_size);

		/**
		 * Maps an area of virtual address space to a set of physical backing frames
		 * @param frames The backing frames to use
		 * @param flags The flags to map the area with
		 * @param page_allocator Allocator to allocate the virtual address area from
		 * @param page_table_allocator Allocator to allocate any necessary page tables from
		 * @return Beginning of the virtual region
		 * @attention Expects the \p frames to be already allocated
		 */
		/*static Page new_map(FrameVector frames,
		                    paging::PageTableEntry::flags_t flags,
		                    IVirtualAllocator& page_allocator,
		                    IPhysicalAllocator& page_table_allocator,
		                    paging::AddressSpace& in = *paging::current_page_table);*/

		/**
		 * Creates a new virtual address space area with at least \p byte_length size
		 * @param byte_length The minimum size of the area to create
		 * @param flags The flags to map the area with
		 * @param page_allocator Allocator to allocate the virtual address area from
		 * @param frame_allocator Allocator to allocate any physical frames from
		 * @param page_table_allocator Allocator to allocate any necessary page tables from
		 * @return Beginning of the virtual region
		 */
		/*static Page new_anonymous_map(uint64_t byte_length,
		                              paging::PageTableEntry::flags_t flags,
		                              IVirtualAllocator& page_allocator,
		                              IPhysicalAllocator& frame_allocator,
		                              IPhysicalAllocator& page_table_allocator,
		                              paging::AddressSpace& in = *paging::current_page_table);*/

		/**
		 * Wrapper around new_map()
		 * Maps an area of virtual address space to the contiguous memory area specified by \p start and \p byte_length
		 * @param start The beginning of the physical memory area
		 * @param byte_length The size of the area to map
		 * @return Beginning of the virtual region
		 * @attention Expects the physical area to be already allocated
		 */
		/*static VirtualAddress new_address_map(PhysicalAddress start,
		                                      uint64_t byte_length,
		                                      paging::PageTableEntry::flags_t flags,
		                                      IVirtualAllocator& page_allocator,
		                                      IPhysicalAllocator& page_table_allocator,
		                                      paging::AddressSpace& in = *paging::current_page_table);*/

	private:
		PhysicalRegion backing_region{};
		VirtualRegion<VAllocator> virtual_region{};
	};
}   // namespace memory

#endif   // HUGOS_MEMORY_MAPPER_HPP
