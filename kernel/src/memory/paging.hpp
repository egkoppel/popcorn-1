/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_PAGING_HPP
#define HUGOS_PAGING_HPP

#include "physical_allocator.hpp"
#include "types.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <log.hpp>
#include <optional>
#include <type_traits>
#include <utils.h>

/*
 * NOTE
 * All types must be declared before including <arch/paging.hpp>
 * All concept assertions must be declared after including
 */

namespace memory::paging {
	enum class PageTableFlags : u64 {
		// Generic flags
		PRESENT    = 1ull << 0,
		WRITEABLE  = 1ull << 1,
		USER       = 1ull << 2,
		ACCESSED   = 1ull << 5,
		DIRTY      = 1ull << 6,
		GLOBAL     = 1ull << 8,
		AVL1       = 1ull << 9,
		AVL2       = 1ull << 10,
		AVL3       = 1ull << 11,
		NO_EXECUTE = 1ull << 63,

		// Kernel use bits
		NO_MAP = AVL1,

		// Implementation specific bits
		IMPL_CACHE_WRITETHROUGH = 1ull << 3,
		IMPL_CACHE_DISABLE      = 1ull << 4,
		IMPL_FLAG_BITS          = NO_EXECUTE | 0xfffull,
		IMPL_ADDR_BITS          = ~IMPL_FLAG_BITS
	};
	constexpr PageTableFlags operator|(PageTableFlags lhs, PageTableFlags rhs) {
		using t = std::underlying_type_t<PageTableFlags>;
		return static_cast<PageTableFlags>(static_cast<t>(lhs) | static_cast<t>(rhs));
	}
	constexpr PageTableFlags operator&(PageTableFlags lhs, PageTableFlags rhs) {
		using t = std::underlying_type_t<PageTableFlags>;
		return static_cast<PageTableFlags>(static_cast<t>(lhs) & static_cast<t>(rhs));
	}
	constexpr PageTableFlags operator~(PageTableFlags arg) {
		using t = std::underlying_type_t<PageTableFlags>;
		return static_cast<PageTableFlags>(~static_cast<t>(arg));
	}

	class PageTableEntryImpl {
		friend class AddressSpaceBase;

	protected:
		using data_t = u64;

		PageTableEntryImpl() noexcept : data{0} {}
		explicit PageTableEntryImpl(data_t data) noexcept : data(data) {}
		data_t data;

		void set_pointed_frame(const frame_t *) noexcept;

	public:
		std::optional<frame_t *> pointed_frame() noexcept;
		std::optional<const frame_t *> pointed_frame() const noexcept;
		PageTableFlags get_flags() const noexcept;
		void set_flags(PageTableFlags) noexcept;
	};

	template<std::size_t Level> class PageTable;

	template<std::size_t Level> class PageTableEntry;
	template<std::size_t Level> requires(Level <= 4 && Level >= 2)
	class PageTableEntry<Level> : public PageTableEntryImpl {
	public:
		//****** NOTE: Don't move these to paging.ipp since the templating stuff breaks ******

		explicit PageTableEntry() noexcept : PageTableEntryImpl{0} {}
		PageTableEntry(const PageTableEntry&) = delete;

		explicit PageTableEntry(const PageTableEntry& other, IPhysicalAllocator& allocator, deep_copy_t)
			: PageTableEntryImpl(other.data) {
			if (auto page_table = other.child_table()) {
				auto new_table = new (&allocator) PageTable<Level - 1>{*page_table.value(), allocator, deep_copy};
				aligned<vaddr_t> new_table_vaddr = vaddr_t{.address = reinterpret_cast<usize>(new_table)};
				const frame_t *frame             = new_table_vaddr.page_map_region_to_frame();
				this->set_pointed_frame(frame);
			}
		}

		~PageTableEntry() { this->drop_child(); }

		PageTableEntry& operator=(const PageTableEntry& rhs) noexcept {
			this->drop_child();
			this->data = rhs.data;
			if (auto pf = this->pointed_frame()) pf->ref_count++;
			return *this;
		}

		std::optional<PageTable<Level - 1> *> child_table() noexcept {
			LOG(Log::TRACE, "Request child table - data: %llb", this->data);

			return this->pointed_frame().and_then([](auto f) {
				return std::optional{static_cast<PageTable<Level - 1> *>(f->frame_to_page_map_region().address)};
			});
		}
		std::optional<const PageTable<Level - 1> *> child_table() const noexcept {
			LOG(Log::TRACE, "Request child table - data: %llb", this->data);

			return this->pointed_frame().and_then([](auto f) {
				return std::optional{static_cast<const PageTable<Level - 1> *>(f->frame_to_page_map_region().address)};
			});
		}
		PageTable<Level - 1> *child_table_or_create(IPhysicalAllocator *allocator) {
			return this->child_table()
			        .or_else([=] {
						LOG(Log::TRACE, "Allocate new page level");

						auto new_pt       = new (allocator) PageTable<Level - 1>{};
						auto new_pt_frame = aligned<paddr_t>{
								vaddr_t{.address = reinterpret_cast<usize>(new_pt)}.devirtualise()};
						this->set_pointed_frame(new_pt_frame.frame());
						this->set_flags(PageTableFlags::PRESENT | PageTableFlags::WRITEABLE | PageTableFlags::USER);
						return new_pt;
					})
			        .value();
		}

	private:
		void drop_child() {
			if (auto page_table = this->child_table()) {
				aligned<vaddr_t> page_table_address = vaddr_t{.address = reinterpret_cast<usize>(page_table.value())};
				frame_t *frame                      = page_table_address.page_map_region_to_frame();

				if (frame->ref_count == 1) {
					// page_table.value()->~PageTable<Level - 1>();
					// IPhysicalAllocator::drop(frame, constants::frame_size);
				}
			}
		}
	};

	template<> class PageTableEntry<1> : public PageTableEntryImpl {
	public:
		//****** NOTE: Don't move these to paging.ipp since the templating stuff breaks ******

		explicit PageTableEntry() noexcept : PageTableEntryImpl{0} {}
		PageTableEntry(const PageTableEntry&) = delete;

		explicit PageTableEntry(const PageTableEntry& other, IPhysicalAllocator& allocator, deep_copy_t);

		/*PageTableEntry& operator=(const PageTableEntry& rhs) noexcept {
		    this->data = rhs.data;
		    return *this;
		}*/
	};

	template<std::size_t Level> class PageTable {
	public:
		using iterator       = PageTableEntry<Level> *;
		using const_iterator = const PageTableEntry<Level> *;

		PageTable()                 = default;
		PageTable(const PageTable&) = delete;

		explicit PageTable(const PageTable& other, IPhysicalAllocator& allocator, deep_copy_t);

		/**
		 * Allocates physical and virtual memory for the page table using the given physical memory allocator
		 * @return Virtual pointer to the location of the new page table
		 */
		void *operator new(std::size_t size, IPhysicalAllocator *allocator);

		void operator delete(void *address, IPhysicalAllocator *) noexcept;

		PageTableEntry<Level>& operator[](std::size_t) noexcept;
		const PageTableEntry<Level>& operator[](std::size_t) const noexcept;

		int print_to(FILE *f, uint64_t addr) requires(Level >= 2);
		int print_to(FILE *f, uint64_t addr) requires(Level == 1);

		decltype(auto) begin() {
			using std::begin;
			return begin(this->entries);
		}

		decltype(auto) end() {
			using std::end;
			return end(this->entries);
		}

		decltype(auto) cbegin() const {
			using std::cbegin;
			return cbegin(this->entries);
		}

		decltype(auto) cend() const {
			using std::cend;
			return cend(this->entries);
		}

	private:
		PageTableEntry<Level> entries[512];
	};

	static_assert(sizeof(PageTable<4>) == 4096);
	static_assert(sizeof(PageTable<3>) == 4096);
	static_assert(sizeof(PageTable<2>) == 4096);
	static_assert(sizeof(PageTable<1>) == 4096);

	class AddressSpaceBase {
	public:
		/**
		 * Creates a new, empty address space
		 */
		explicit AddressSpaceBase(PageTable<4> *l4_table, IPhysicalAllocator *allocator = &allocators.general())
			: l4_table{l4_table},
			  allocator{allocator} {}

		AddressSpaceBase(const AddressSpaceBase&) = delete;
		AddressSpaceBase(AddressSpaceBase&&);

		/*~AddressSpaceBase() {
		    if (atomic_fetch_sub(this->ref_count, 1) == 1) {
		        this->l4_table->~PageTable<4>();
		        PageTable<4>::operator delete(this->l4_table, this->allocator);
		        delete this->ref_count;
		    }
		}*/

		void map_page_to(aligned<vaddr_t> page, const frame_t *frame, PageTableFlags flags);
		bool unmap_page(aligned<vaddr_t> page);
		int print_to(FILE *f) { return this->l4_table->print_to(f, 0); }
		void make_active();
		std::optional<frame_t *> translate_page(aligned<vaddr_t> page);
		frame_t *l4_table_frame() {
			return aligned<vaddr_t>{vaddr_t{.address = reinterpret_cast<usize>(this->l4_table)}}
			        .page_map_region_to_frame();
		}
		const frame_t *l4_table_frame() const {
			LOG(Log::DEBUG, "%lp %lp", this->l4_table, reinterpret_cast<usize>(this->l4_table));
			auto a = aligned<vaddr_t>{vaddr_t{.address = reinterpret_cast<usize>(this->l4_table)}};
			return a.page_map_region_to_frame();
		}

		void rebind_allocator(IPhysicalAllocator& allocator) { this->allocator = &allocator; }

	protected:
		PageTable<4> *l4_table;          //!< Pointer to the level 4 page table for this address space
		IPhysicalAllocator *allocator;   //!< Allocator to use when adding/removing page tables
	};

	class AddressSpace : public AddressSpaceBase {
	public:
		/**
		 * Creates a new, empty address space
		 */
		explicit AddressSpace(IPhysicalAllocator& allocator = allocators.general());

		AddressSpace(AddressSpace&&);

		~AddressSpace() {
			if (atomic_fetch_sub(this->ref_count, 1) == 1) {
				this->l4_table->~PageTable<4>();
				PageTable<4>::operator delete(this->l4_table, this->allocator);
				delete this->ref_count;
			}
		}

		/**
		 * Performs an explicit shallow copy of the address space, to create a shared address space
		 * eg. when creating a new thread
		 */
		explicit AddressSpace(const AddressSpace& other, shallow_copy_t) noexcept
			: AddressSpaceBase(other.l4_table, other.allocator),
			  ref_count{other.ref_count} {
			atomic_fetch_add(this->ref_count, 1);
		}

		/**
		 * Performs an explicit deep copy of the address space, to create a duplicated, but separate, address space
		 * eg. when fork is called
		 */
		explicit AddressSpace(const AddressSpace& other, deep_copy_t, IPhysicalAllocator& allocator)
			: AddressSpaceBase(new(&allocator) PageTable<4>{*other.l4_table, allocator, deep_copy}, &allocator),
			  ref_count{new atomic_uint_fast64_t(1)} {}

		inline explicit AddressSpace(const AddressSpace& other, deep_copy_t)
			: AddressSpace{other, deep_copy, *other.allocator} {}

	private:
		atomic_uint_fast64_t *ref_count;   //!< Number of users of this address space
	};

	class KernelAddressSpace : public AddressSpaceBase {
		friend class AddressSpace;

	public:
		KernelAddressSpace(PageTable<4> *l4_table, IPhysicalAllocator& allocator)
			: AddressSpaceBase(l4_table, &allocator) {}
	};

	extern KernelAddressSpace& kas;
	inline KernelAddressSpace& init_kas(IPhysicalAllocator& allocator) {
		new (&kas) KernelAddressSpace{new (&allocator) PageTable<4>{}, allocator};
		return kas;
	}

	class AlreadyMappedException : public std::exception {
	public:
		AlreadyMappedException() = default;
		const char *what() const noexcept override { return "AlreadyMappedException"; }
	};

}   // namespace memory::paging

#include <arch/paging.hpp>

namespace memory::paging::detail {}

#include "paging.ipp"

#endif   // HUGOS_PAGING_HPP
