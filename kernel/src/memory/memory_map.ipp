
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_MEMORY_MEMORY_MAP_IPP
#define POPCORN_KERNEL_SRC_MEMORY_MEMORY_MAP_IPP

namespace memory {
	namespace detail {
		template<class VAllocator>
		MemoryMapBase<VAllocator>::MemoryMapBase(PhysicalRegion&& backing_region,
		                                         paging::PageTableFlags flags,
		                                         IPhysicalAllocator& page_allocator,
		                                         paging::AddressSpaceBase& in,
		                                         VAllocator allocator)
			: backing_region(std::move(backing_region)),
			  virtual_region(this->backing_region.size(), allocator),
			  address_space(&in) {
			for (auto [page, frame] : iter::zip(this->virtual_region, this->backing_region)) {
				in.map_page_to(page, frame, flags);
			}
		}

		template<class VAllocator> MemoryMapBase<VAllocator>::~MemoryMapBase() {
			for (auto page : this->virtual_region) { this->address_space->unmap_page(page); }
		}


		template<class VAllocator>
		MemoryMapBase<VAllocator>::MemoryMapBase(MemoryMapBase&& other) noexcept
			: backing_region(std::move(other.backing_region)),
			  virtual_region(std::move(other.virtual_region)),
			  address_space(other.address_space) {}

		template<class VAllocator>
		MemoryMapBase<VAllocator>& MemoryMapBase<VAllocator>::operator=(MemoryMapBase&& rhs) noexcept {
			using std::swap;
			swap(this->backing_region, rhs.backing_region);
			swap(this->virtual_region, rhs.virtual_region);
			swap(this->address_space, rhs.address_space);
			return *this;
		}

		template<class VAllocator> void MemoryMapBase<VAllocator>::resize_to(std::size_t new_size) {
			if (new_size > this->size()) THROW("i'll fix it later exception");
		}
	}   // namespace detail

	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(usize byte_count,
	                                    paging::PageTableFlags flags,
	                                    IPhysicalAllocator& page_allocator,
	                                    paging::AddressSpaceBase& in,
	                                    VAllocator allocator)
		: detail::MemoryMapBase<VAllocator>(PhysicalRegion(byte_count, page_allocator),
	                                        flags,
	                                        page_allocator,
	                                        in,
	                                        allocator),
		  data(reinterpret_cast<T *>((*this->virtual_region.begin()).address.address)) {}

	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(paddr_t at,
	                                    usize byte_count,
	                                    paging::PageTableFlags flags,
	                                    IPhysicalAllocator& page_allocator,
	                                    paging::AddressSpaceBase& in,
	                                    VAllocator allocator)
		: detail::MemoryMapBase<VAllocator>(PhysicalRegion(aligned<paddr_t>::aligned_down(at),
	                                                       calculate_total_size(at, byte_count),
	                                                       page_allocator),
	                                        flags,
	                                        page_allocator,
	                                        in,
	                                        allocator),
		  data(reinterpret_cast<T *>((*this->virtual_region.begin()).address.address + calculate_offset(at))) {}

	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(MemoryMap&& other) noexcept
		: detail::MemoryMapBase<VAllocator>(static_cast<detail::MemoryMapBase<VAllocator>&&>(other)),
		  data(other.data) {}

	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(detail::MemoryMapBase<VAllocator>&& other, T *data) noexcept
		: detail::MemoryMapBase<VAllocator>(std::move(other)),
		  data(data) {}

	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>& MemoryMap<T, VAllocator>::operator=(MemoryMap&& rhs) noexcept {
		using std::swap;
		swap(this->backing_region, rhs.backing_region);
		swap(this->virtual_region, rhs.virtual_region);
		swap(this->address_space, rhs.address_space);
		this->data = rhs.data;
		return *this;
	}

	template<class T, class VAllocator>
	std::size_t constexpr MemoryMap<T, VAllocator>::calculate_total_size(paddr_t start, std::size_t requested_size) {
		return requested_size + calculate_offset(start);
	}
	template<class T, class VAllocator>
	std::size_t constexpr MemoryMap<T, VAllocator>::calculate_offset(paddr_t start) {
		return (start.address - aligned<paddr_t>::aligned_down(start).address.address);
	}

	template<class T, class U, class VAllocator> requires(requires { static_cast<T *>((U *)nullptr); })
	MemoryMap<T, VAllocator> static_pointer_cast(MemoryMap<U, VAllocator>&& r) {
		auto new_ptr = static_cast<T *>(r.data);
		return {static_cast<detail::MemoryMapBase<VAllocator>&&>(r), new_ptr};
	}
	template<class T, class U, class VAllocator>
	MemoryMap<T, VAllocator> dynamic_pointer_cast(MemoryMap<U, VAllocator>&& r) {
		auto new_ptr = dynamic_cast<T *>(r.data);
		return {static_cast<detail::MemoryMapBase<VAllocator>&&>(r), new_ptr};
	}
	template<class T, class U, class VAllocator>
	MemoryMap<T, VAllocator> const_pointer_cast(MemoryMap<U, VAllocator>&& r) {
		auto new_ptr = const_cast<T *>(r.data);
		return {static_cast<detail::MemoryMapBase<VAllocator>&&>(r), new_ptr};
	}
	template<class T, class U, class VAllocator>
	MemoryMap<T, VAllocator> reinterpret_pointer_cast(MemoryMap<U, VAllocator>&& r) {
		auto new_ptr = reinterpret_cast<T *>(r.data);
		return {static_cast<detail::MemoryMapBase<VAllocator>&&>(r), new_ptr};
	}
}   // namespace memory

#endif   // POPCORN_KERNEL_SRC_MEMORY_MEMORY_MAP_IPP
