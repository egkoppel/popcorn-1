
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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
	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(usize byte_count,
	                                    paging::PageTableFlags flags,
	                                    IPhysicalAllocator& page_allocator,
	                                    paging::AddressSpaceBase& in,
	                                    VAllocator allocator) :
		backing_region(byte_count, page_allocator),
		virtual_region(byte_count, allocator),
		address_space(&in) {
		for (auto [page, frame] : iter::zip(this->virtual_region, this->backing_region)) {
			in.map_page_to(page, frame, flags);
		}
	}

	template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(paddr_t at,
	                                    usize byte_count,
	                                    paging::PageTableFlags flags,
	                                    IPhysicalAllocator& page_allocator,
	                                    paging::AddressSpaceBase& in,
	                                    VAllocator allocator) :
		backing_region(at - calculate_offset(at), calculate_total_size(at, byte_count), page_allocator),
		virtual_region(calculate_total_size(at, byte_count), allocator),
		address_space(&in),
		offset(calculate_offset(at)) {
		for (auto [page, frame] : iter::zip(this->virtual_region, this->backing_region)) {
			in.map_page_to(page, frame, flags);
		}
	}

	template<class T, class VAllocator> MemoryMap<T, VAllocator>::~MemoryMap() {
		for (auto page : this->virtual_region) { this->address_space->unmap_page(page); }
	}

	template<class T, class VAllocator> MemoryMap<T, VAllocator>::MemoryMap(MemoryMap&& other) :
		backing_region(std::move(other.backing_region)),
		virtual_region(std::move(other.virtual_region)),
		address_space(other.address_space),
		offset(other.offset) {}

	template<class T, class VAllocator>
	std::size_t constexpr MemoryMap<T, VAllocator>::calculate_total_size(paddr_t start, std::size_t requested_size) {
		return requested_size + calculate_offset(start);
	}
	template<class T, class VAllocator>
	std::size_t constexpr MemoryMap<T, VAllocator>::calculate_offset(paddr_t start) {
		return (start.address - aligned<paddr_t>::aligned_down(start).address.address);
	}
}   // namespace memory

#endif   //POPCORN_KERNEL_SRC_MEMORY_MEMORY_MAP_IPP
