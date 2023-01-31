/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "paging.hpp"

#include <algorithm>
#include <limits>
#include <log.hpp>
#include <optional>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <utility/zip.hpp>
#include <utils.h>

namespace memory::paging {
	void PageTableEntryImpl::set_pointed_frame(const frame_t *frame) noexcept {
		LOG(Log::TRACE, "pfn: %zu", frame->number());
		this->data = (this->data & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_FLAG_BITS))
		             | ((frame->addr())
		                & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_ADDR_BITS));
		LOG(Log::TRACE, "l1 entry data %llb", this->data);
	}

	std::optional<frame_t *> PageTableEntryImpl::pointed_frame() noexcept {
		return const_cast<const PageTableEntryImpl *>(this)->pointed_frame().and_then([](auto f) {
			return std::optional{const_cast<frame_t *>(f)};
		});
	}
	std::optional<const frame_t *> PageTableEntryImpl::pointed_frame() const noexcept {
		if ((this->data & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::PRESENT)) == 0)
			return std::nullopt;
		auto frame_addr = this->data
		                  & static_cast<std::underlying_type_t<PageTableFlags>>(PageTableFlags::IMPL_ADDR_BITS);
		auto frame_num = frame_addr / constants::frame_size;
		LOG(Log::TRACE, "pfn: %lu", frame_num);
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
		LOG(Log::DEBUG, "map %llx to pfn %zu", page, frame->number());

		auto l1_table = (*(*(*this->l4_table)[page.address.page_table_index<4>()]
		                            .child_table_or_create(this->allocator))[page.address.page_table_index<3>()]
		                          .child_table_or_create(this->allocator))[page.address.page_table_index<2>()]
		                        .child_table_or_create(this->allocator);
		decltype(auto) l1_entry = (*l1_table)[page.address.page_table_index<1>()];
		if (static_cast<bool>(l1_entry.get_flags() & PageTableFlags::PRESENT)) THROW(AlreadyMappedException());
		l1_entry.set_flags(flags | PageTableFlags::PRESENT);
		l1_entry.set_pointed_frame(frame);

		LOG(Log::DEBUG, "mapped %llx to pfn %zu", page, frame->number());
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

		LOG(Log::DEBUG, "unmapped %llx from pfn %zu", page, l1_entry.pointed_frame().value()->number());
		l1_entry.set_flags(flags & ~PageTableFlags::PRESENT);
		__asm__ volatile("invlpg %0" : : "m"(page));

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

	AddressSpace::AddressSpace(IPhysicalAllocator& allocator)
		: AddressSpaceBase(new(&allocator) PageTable<4>, &allocator),
		  ref_count(new atomic_uint_fast64_t(1)) {
		for (usize i = 256; i < 512; i++) { (*this->l4_table)[i] = (*kas.l4_table)[i]; }
	}
}   // namespace memory::paging
