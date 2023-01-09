
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_MEMORY_PAGING_IPP
#define POPCORN_KERNEL_SRC_MEMORY_PAGING_IPP

namespace memory::paging {
	template<std::size_t Level>
	PageTable<Level>::PageTable(const PageTable& other, IPhysicalAllocator& allocator, deep_copy_t) {
		for (size_t i = 0; i < 512; ++i) {
			this->entries[i] = PageTableEntry<Level>{other.entries[i], allocator, deep_copy};   // Deep-copy the entries
		}
	}

	template<std::size_t Level> void *PageTable<Level>::operator new(std::size_t size, IPhysicalAllocator *allocator) {
		if (size != constants::frame_size) THROW(std::bad_alloc());
		vaddr_t page_table_address = allocator->allocate(size)->frame_to_page_map_region();
		return static_cast<void *>(page_table_address);
	}

	template<std::size_t Level> void PageTable<Level>::operator delete(void *address, IPhysicalAllocator *) noexcept {
		aligned<vaddr_t> page_table_address = vaddr_t{.address = reinterpret_cast<usize>(address)};
		frame_t *frame                      = page_table_address.page_map_region_to_frame();
		IPhysicalAllocator::drop(frame, constants::frame_size);
	}

	template<std::size_t Level> PageTableEntry<Level>& PageTable<Level>::operator[](std::size_t index) noexcept {
		return this->entries[index];
	}
	template<std::size_t Level>
	const PageTableEntry<Level>& PageTable<Level>::operator[](std::size_t index) const noexcept {
		return this->entries[index];
	}

	template<std::size_t Level>
	int PageTable<Level>::print_to(FILE *f, u64 addr) requires(Level >= 2)
	{
		int char_count = 0;
		for (size_t i = 0; i < 512; ++i) {
			uint64_t this_addr = addr;
			this_addr |= i << ((Level - 1) * 9 + 12);
			if (auto child = (*this)[i].child_table()) {
				char_count += child.value()->print_to(f, this_addr);
			} else {
				char_count += fprintf(f, "%lp... -> (nil)\n", this_addr);
			}
		}
		return char_count;
	}

	template<std::size_t Level>
	int PageTable<Level>::print_to(FILE *f, u64 addr) requires(Level == 1)
	{
		int char_count = 0;
		for (size_t i = 0; i < 512; ++i) {
			uint64_t this_addr = addr;
			this_addr |= i << ((Level - 1) * 9 + 12);
			auto& entry = (*this)[i];
			if (static_cast<bool>(entry.get_flags() & PageTableFlags::PRESENT)) {
				char_count += fprintf(f,
				                      "%lp -> %lp, %c%c%c%c\n",
				                      this_addr,
				                      entry.pointed_frame()->addr(),
				                      static_cast<bool>(entry.get_flags() & PageTableFlags::WRITEABLE) ? 'W' : 'R',
				                      static_cast<bool>(entry.get_flags() & PageTableFlags::USER) ? 'U' : 'S',
				                      static_cast<bool>(entry.get_flags() & PageTableFlags::GLOBAL) ? 'G' : '-',
				                      static_cast<bool>(entry.get_flags() & PageTableFlags::NO_EXECUTE) ? '-' : 'X');
			} else {
				char_count += fprintf(f, "%lp -> (nil)\n", this_addr);
			}
		}
		return char_count;
	}
}   // namespace memory::paging

#endif   // POPCORN_KERNEL_SRC_MEMORY_PAGING_IPP
