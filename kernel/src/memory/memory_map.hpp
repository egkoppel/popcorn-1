/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
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

#include <cstddef>
#include <optional>
#include <utils.h>

namespace memory {
	namespace detail {
		template<class VAllocator = general_allocator_t> class MemoryMapBase {
		protected:
			MemoryMapBase() = default;
			/**
			 * Creates an anonymous memory mapping
			 * @param byte_count
			 * @param flags
			 * @param page_allocator
			 * @param in
			 */
			MemoryMapBase(PhysicalRegion&& backing_region,
			              paging::PageTableFlags flags,
			              IPhysicalAllocator& page_allocator,
			              paging::AddressSpaceBase& in,
			              VAllocator allocator = VAllocator());
			MemoryMapBase(const MemoryMapBase&) = delete;
			MemoryMapBase(MemoryMapBase&&) noexcept;
			~MemoryMapBase();

			MemoryMapBase& operator=(MemoryMapBase&&) noexcept;

		public:
			void resize_to(std::size_t new_size);
			std::size_t size() { return this->virtual_region.size(); }
			vaddr_t start() { return *this->virtual_region.begin(); }
			vaddr_t end() { return *this->virtual_region.end(); }
			paddr_t pstart() { return paddr_t{.address = (*this->backing_region.begin())->addr()}; }
			paddr_t pend() { return paddr_t{.address = (*this->backing_region.end())->addr()}; }

		protected:
			PhysicalRegion backing_region;
			VirtualRegion<VAllocator> virtual_region;
			paging::AddressSpaceBase *address_space = nullptr;
		};
	}   // namespace detail

	/**
	 * @brief A mapping between virtual and physical memory
	 * @tparam VAllocator The virtual allocator to use
	 * @tparam T The type of the underlying object
	 *
	 * A MemoryMap represents a RAII-based map between virtual and physical memory space, and should usually be treated
	 * similarly to a pointer by specifying the type of \p T.
	 */
	template<class T = std::byte, class VAllocator = general_allocator_t>
	class MemoryMap : public detail::MemoryMapBase<VAllocator> {
	public:
		using detail::MemoryMapBase<VAllocator>::size;
		using detail::MemoryMapBase<VAllocator>::resize_to;

		MemoryMap() = default;

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
		          paging::AddressSpaceBase& in,
		          VAllocator allocator = VAllocator());

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
		          paging::AddressSpaceBase& in,
		          VAllocator allocator = VAllocator());
		MemoryMap(const MemoryMap&) = delete;

		MemoryMap(MemoryMap&&) noexcept;
		MemoryMap& operator=(MemoryMap&&) noexcept;

		T& operator*() { return *this->data; }
		T *operator->() { return this->data; }
		T *get() { return this->data; }
		T& operator[](std::size_t offset) { return this->data[offset]; }
		const T& operator*() const { return *this->data; }
		const T *operator->() const { return this->data; }
		const T *get() const { return this->data; }
		const T& operator[](std::size_t offset) const { return this->data[offset]; }

		template<class Tp, class U, class VAllocatorp> requires(requires { static_cast<Tp *>((U *)nullptr); })
		friend MemoryMap<Tp, VAllocatorp> static_pointer_cast(MemoryMap<U, VAllocatorp>&& r);
		template<class Tp, class U, class VAllocatorp> requires(requires { dynamic_cast<Tp *>((U *)nullptr); })
		friend MemoryMap<Tp, VAllocatorp> dynamic_pointer_cast(MemoryMap<U, VAllocatorp>&& r);
		template<class Tp, class U, class VAllocatorp> requires(requires { const_cast<Tp *>((U *)nullptr); })
		friend MemoryMap<Tp, VAllocatorp> const_pointer_cast(MemoryMap<U, VAllocatorp>&& r);
		template<class Tp, class U, class VAllocatorp> requires(requires { reinterpret_cast<Tp *>((U *)nullptr); })
		friend MemoryMap<Tp, VAllocatorp> reinterpret_pointer_cast(MemoryMap<U, VAllocatorp>&& r);

	private:
		MemoryMap(detail::MemoryMapBase<VAllocator>&&, T *data) noexcept;
		T *data = nullptr;

		std::size_t constexpr calculate_total_size(paddr_t start, std::size_t requested_size);
		std::size_t constexpr calculate_offset(paddr_t start);
	};

	template<class VAllocator> class MemoryMap<void, VAllocator> : public detail::MemoryMapBase<VAllocator> {
	public:
		using detail::MemoryMapBase<VAllocator>::size;
		using detail::MemoryMapBase<VAllocator>::resize_to;
		using detail::MemoryMapBase<VAllocator>::operator=;

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
		          paging::AddressSpaceBase& in,
		          VAllocator allocator = VAllocator());
	};
}   // namespace memory

#include "memory_map.ipp"

#endif   // HUGOS_MEMORY_MAPPER_HPP
