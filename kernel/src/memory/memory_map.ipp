
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
		backing_region{byte_count, page_allocator},
		virtual_region{allocator},
		address_space{&in} {
		for (auto [page, frame] : iter::zip(this->virtual_region, this->backing_region)) {
			in.map_page_to(page, frame, flags);
		}
	}

	/*template<class T, class VAllocator>
	MemoryMap<T, VAllocator>::MemoryMap(paddr_t at,
	                                    usize byte_count,
	                                    paging::PageTableFlags flags,
	                                    IPhysicalAllocator& page_allocator,
	                                    paging::AddressSpaceBase& in,
	                                    VAllocator allocator) {}*/

	template<class T, class VAllocator> MemoryMap<T, VAllocator>::~MemoryMap() {
		for (auto page : this->virtual_region) { this->address_space->unmap_page(page); }
	}
}   // namespace memory

#endif   //POPCORN_KERNEL_SRC_MEMORY_MEMORY_MAP_IPP
