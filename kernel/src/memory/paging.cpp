/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "paging.hpp"

#include <algorithm>
#include <log.hpp>
#include <optional>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <utility/zip.hpp>
#include <utils.h>

/*template<size_t Level> struct PageTableEntry {
private:
    enum {
        PRESENT                = 0,
        WRITEABLE              = 1,
        USER                   = 2,
        WRITE_THROUGH          = 3,
        CACHE_DISABLE          = 4,
        ACCESSED               = 5,
        DIRTY                  = 6,
        PAGE_ATTR_TABLE_LEVEL1 = 7,
        PAGE_SIZE              = 7,
        GLOBAL                 = 8,
        PAGE_ATTR_TABLE        = 12,
        NX                     = 63
    };

    uint64_t data;

public:
    inline bool get_present() const { return this->data & (1 << PRESENT); }
    inline bool get_writeable() const { return this->data & (1 << WRITEABLE); }
    inline bool get_user() const { return this->data & (1 << USER); }
    inline bool get_write_through() const { return this->data & (1 << WRITE_THROUGH); }
    inline bool get_cache_disable() const { return this->data & (1 << CACHE_DISABLE); }
    inline bool get_accessed() const { return this->data & (1 << ACCESSED); }
    inline bool get_dirty() const { return this->data & (1 << DIRTY); }
    inline bool get_pat() const
        requires(Level == 1)
    {
        return this->data & (1 << PAGE_ATTR_TABLE_LEVEL1);
    }
    inline bool get_pat() const
        requires(Level > 1)
    {
        return this->data & (1 << PAGE_ATTR_TABLE);
    }
    inline bool get_page_size() const
        requires(Level == 2 || Level == 3)
    {
        return this->data & (1 << PAGE_SIZE);
    }
    inline bool get_global() const { return this->data & (1 << GLOBAL); }
    inline bool get_nx() const { return this->data & (1 << NX); }
    inline std::optional<uint64_t> get_addr() const { return Some(this->data & 0xFFFFFFFFFF000); };

    inline void set_present(bool val) { this->data = (this->data & ~(1 << PRESENT)) | val << PRESENT; }
    inline void set_writeable(bool val) { this->data = (this->data & ~(1 << WRITEABLE)) | val << WRITEABLE; }
    inline void set_user(bool val) { this->data = (this->data & ~(1 << USER)) | val << USER; }
    inline void set_write_through(bool val) {
        this->data = (this->data & ~(1 << WRITE_THROUGH)) | val << WRITE_THROUGH;
    }
    inline void set_cache_disable(bool val) {
        this->data = (this->data & ~(1 << CACHE_DISABLE)) | val << CACHE_DISABLE;
    }
    inline void set_accessed(bool val) { this->data = (this->data & ~(1 << ACCESSED)) | val << ACCESSED; }
    inline void set_dirty(bool val) { this->data = (this->data & ~(1 << DIRTY)) | val << DIRTY; }
    inline void set_pat(bool val)
        requires(Level == 1)
    {
        this->data = (this->data & ~(1 << PAGE_ATTR_TABLE_LEVEL1)) | val << PAGE_ATTR_TABLE_LEVEL1;
    }
    inline void set_pat(bool val)
        requires(Level > 1)
    {
        this->data = (this->data & ~(1 << PAGE_ATTR_TABLE)) | val << PAGE_ATTR_TABLE;
    }
    inline void set_page_size(bool val)
        requires(Level == 2 || Level == 3)
    {
        this->data = (this->data & ~(1 << PAGE_SIZE)) | val << PAGE_SIZE;
    }
    inline void set_global(bool val) { this->data = (this->data & ~(1 << GLOBAL)) | val << GLOBAL; }
    inline void set_nx(bool val) { this->data = (this->data & ~(1 << NX)) | val << NX; }
    [[nodiscard]] inline int set_addr(uint64_t val) {
        if ((val & ~0xFFFFFFFFFF000) != 0) return -1;
        this->data = (this->data & ~0xFFFFFFFFFF000) | val;
        return 0;
    };

    inline void clear() { this->data = 0; }
};

template<size_t Level> struct PageTable {
private:
    PageTableEntry<Level> entries[512];

public:
    PageTableEntry<Level>& operator[](size_t index) { return this->entries[index]; }

    std::optional<PageTable<Level - 1> *> get_child_table(size_t index)
        requires(Level > 1)
    {
        if (this[index].get_present() && !this[index].get_page_size()) {
            return Some(
                    reinterpret_cast<PageTable<Level - 1> *>((reinterpret_cast<uint64_t>(this) << 9) | (index << 12)));
        }
        return std::nullopt;
    }
};

static_assert(sizeof(PageTable<1>) == 0x1000);*/

namespace memory::paging {
	void PageTableEntryImpl::set_pointed_frame(const frame_t *frame) noexcept {
		LOG(Log::TRACE, "pfn: %d", frame->number());
		this->data = (this->data & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_FLAG_BITS))
		             | ((frame->addr())
		                & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_ADDR_BITS));
		LOG(Log::TRACE, "l1 entry data %llb", this->data);
	}

	frame_t *PageTableEntryImpl::pointed_frame() noexcept {
		return const_cast<frame_t *>(const_cast<const PageTableEntryImpl *>(this)->pointed_frame());
	}
	const frame_t *PageTableEntryImpl::pointed_frame() const noexcept {
		auto frame_addr = this->data
		                  & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_ADDR_BITS);
		auto frame_num = frame_addr / constants::frame_size;
		LOG(Log::TRACE, "pfn: %d", frame_num);
		return &mem_map[frame_num];
	}
	PageTableFlags PageTableEntryImpl::get_flags() const noexcept {
		return static_cast<PageTableFlags>(this->data) & PageTableFlags::IMPL_FLAG_BITS;
	}
	void PageTableEntryImpl::set_flags(PageTableFlags flags) noexcept {
		this->data = (this->data & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_ADDR_BITS))
		             | static_cast<std::underlying_type_t<PageTableFlags>>(flags & PageTableFlags::IMPL_FLAG_BITS);
	}

	void AddressSpaceBase::map_page_to(aligned<vaddr_t> page, const frame_t *frame, PageTableFlags flags) {
		LOG(Log::DEBUG, "map %llx to pfn %d", page, frame->number());

		auto l1_table = (*(*(*this->l4_table)[page.address.page_table_index<4>()]
		                            .child_table_or_create(this->allocator))[page.address.page_table_index<3>()]
		                          .child_table_or_create(this->allocator))[page.address.page_table_index<2>()]
		                        .child_table_or_create(this->allocator);
		decltype(auto) l1_entry = (*l1_table)[page.address.page_table_index<1>()];
		if (static_cast<bool>(l1_entry.get_flags() & PageTableFlags::PRESENT)) throw AlreadyMappedException();
		l1_entry.set_flags(flags | PageTableFlags::PRESENT);
		l1_entry.set_pointed_frame(frame);

		LOG(Log::DEBUG, "mapped %llx to pfn %d", page, frame->number());
	}

	bool AddressSpaceBase::unmap_page(aligned<vaddr_t> page) {
		const auto l1_table = (*this->l4_table)[page.address.page_table_index<4>()]
		                              .child_table()
		                              .and_then([&](const auto l3_table) {
										  return (*l3_table)[page.address.page_table_index<3>()].child_table();
									  })
		                              .and_then([&](const auto l2_table) {
										  return (*l2_table)[page.address.page_table_index<2>()].child_table();
									  });

		if (!l1_table) return false;
		auto& l1_entry = (*l1_table.value())[page.address.page_table_index<1>()];

		auto flags = l1_entry.get_flags();
		if (!static_cast<bool>(flags & PageTableFlags::PRESENT)) return false;
		l1_entry.set_flags(flags & ~PageTableFlags::PRESENT);

		return true;
	}

	void AddressSpaceBase::make_active() {
		vaddr_t virt_addr{.address = reinterpret_cast<usize>(this->l4_table)};
		usize phys_addr = virt_addr.devirtualise().address;
		LOG(Log::DEBUG, "Switch page table to %lp", phys_addr);
		__asm__ volatile("mov %0, %%cr3" ::"r"(phys_addr) : "memory");
	}

	std::optional<frame_t *> AddressSpaceBase::translate_page(aligned<vaddr_t> page) {
		return (*this->l4_table)[page.address.page_table_index<4>()]
		        .child_table()
		        .and_then([&](const auto l3_table) {
					return (*l3_table)[page.address.page_table_index<3>()].child_table();
				})
		        .and_then([&](const auto l2_table) {
					return (*l2_table)[page.address.page_table_index<2>()].child_table();
				})
		        .and_then([&](const auto l1_table) {
					return std::optional{(*l1_table)[page.address.page_table_index<1>()].pointed_frame()};
				});
	}

	alignas(alignof(KernelAddressSpace)) u8 kas_[sizeof(KernelAddressSpace)];
	KernelAddressSpace& kas = reinterpret_cast<KernelAddressSpace&>(kas_);

	/*void memory::paging::PageTableRoot::unmap_page(memory::Page, memory::IPhysicalAllocator& page_table_deallocator)
	 * {}*/

	/*AddressSpace AddressSpace::new_table(IPhysicalAllocator& page_table_allocator) {
	auto fs        = page_table_allocator.allocate(Frame::size);
	auto root_phys = fs.begin()->begin();
	auto root      = static_cast<PageTable<4> *>(root_phys.to_virtual());
	new (root) PageTable<4>();
	return {root_phys, page_table_allocator};
}

AddressSpace AddressSpace::make_active() {
	std::swap(*this, this->current_table);
	PhysicalAddress old_p4_table_frame;
	__asm__ volatile("mov %%cr3, %0" : "=r"(old_p4_table_frame));
	__asm__ volatile("mov %0, %%cr3" : : "r"(this->l4_table));
	return {old_p4_table_frame};
}*/
	AddressSpace::AddressSpace(IPhysicalAllocator& allocator)
		: AddressSpaceBase(new(&allocator) PageTable<4>, &allocator),
		  ref_count(new atomic_uint_fast64_t(1)) {
		for (usize i = 128; i < 512; i++) { (*this->l4_table)[i] = (*kas.l4_table)[i]; }
	}
}   // namespace memory::paging
