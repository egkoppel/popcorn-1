/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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
#include <utility/virtual_object.hpp>
#include <utils.h>

/*
 * NOTE
 * All types must be declared before including <arch/paging.hpp>
 * All concept assertions must be declared after including
 */

namespace memory::paging {
	/*namespace detail {
	    template<size_t MaxIndex> class PageTableIndex {
	    public:
	        class out_of_bounds : public std::exception {
	        private:
	            char buf[55] = "PageTableIndex::out_of_bounds - given(xxxx), max(xxxx)";
	        public:
	            out_of_bounds() = delete;
	            out_of_bounds(size_t given) {
	                snprintf(this->buf, 55, "PageTableIndex::out_of_bounds - given(%zu), max(%zu)", given, MaxIndex);
	            }
	            const char *what() const noexcept override {return this->buf; }
	        };
	    private:
	        size_t value;
	    public:
	        explicit(false) PageTableIndex(size_t value) : value(value) {
	            if (value >= MaxIndex) throw out_of_bounds(value);
	        }
	        constexpr explicit(false) operator size_t() const{ return this->value; }
	    };
	}

	template<size_t Level> class PageTableIndex;
	template<> class PageTableIndex<1> : public detail::PageTableIndex<512> { using
	detail::PageTableIndex<512>::PageTableIndex; }; template<> class PageTableIndex<2> : public
	detail::PageTableIndex<512> { using detail::PageTableIndex<512>::PageTableIndex; }; template<> class
	PageTableIndex<3> : public detail::PageTableIndex<512> { using detail::PageTableIndex<512>::PageTableIndex; };
	template<> class PageTableIndex<4> : public detail::PageTableIndex<512> { using
	detail::PageTableIndex<512>::PageTableIndex; };

	template<template<size_t> typename T, size_t Level> concept PageTableConcept = requires(T<Level>& table) {
	                                             //{ table.thing() } -> std::same_as<int>;
	                                             //requires std::is_constructible<T<Level>, int, int, size_t>::value;
	                                                                     };

	template<size_t Level> class PageTableImpl {
	public:

	};

	const constexpr size_t page_table_root_level = 4;

	template<size_t Level> class PageTableEntryImpl {
	private:
	    uint64_t data;
	public:
	    enum flags : uint64_t {
	        // Generic flags
	        PRESENT = 1<<0,
	        WRITEABLE = 1<<1,
	        USER = 1<<2,
	        ACCESSED = 1<<5,
	        DIRTY = 1<<6,
	        GLOBAL = 1<<8,
	        NO_EXECUTE = 1<<63,

	        // Kernel use bits
	        NO_MAP = 1<<9,

	        // Implementation specific bits
	        IMPL_CACHE_WRITETHROUGH = 1<<3,
	        IMPL_CACHE_DISABLE = 1<<4
	    };

	    [[nodiscard]] PhysicalAddress address() const { return PhysicalAddress(this->data & 0xF'FFFF'FFFF'F000); }
	};

	namespace checkers {
	    template<size_t Level> class PageTableConceptChecker {
	    public:
	        constexpr PageTableConceptChecker() {
	            static_assert(PageTableConcept<PageTableImpl, Level>);
	            PageTableConceptChecker<Level-1>();
	        }
	    };

	    template<> class PageTableConceptChecker<0> {
	    public:
	        constexpr PageTableConceptChecker() = default;
	    };

	    static const constexpr auto check = PageTableConceptChecker<4>();
	}

	template<size_t Level> class PageTableEntry : public PageTableEntryImpl<Level> {

	};

	template<size_t Level> class PageTable : public PageTableImpl<Level> {
	public:
	    PageTable<Level-1>* get_child_table(PageTableIndex<Level>) requires(Level > 1);
	};*/

	/*class PageTableEntry {
	private:
	    uint64_t data;

	public:
	    using flags_t = uint64_t;
	    enum flags : flags_t {
	        // Generic flags
	        PRESENT    = 1ull << 0,
	        WRITEABLE  = 1ull << 1,
	        USER       = 1ull << 2,
	        ACCESSED   = 1ull << 5,
	        DIRTY      = 1ull << 6,
	        GLOBAL     = 1ull << 8,
	        NO_EXECUTE = 1ull << 63,

	        // Kernel use bits
	        NO_MAP = 1ull << 9,

	        // Implementation specific bits
	        IMPL_CACHE_WRITETHROUGH = 1ull << 3,
	        IMPL_CACHE_DISABLE      = 1ull << 4
	    };

	    PageTableEntry() : data(0) {}
	    Frame pointed_frame() const {
	        return Frame::containing_address(PhysicalAddress(this->data & 0x000ffffffffff000));
	    }
	    void point_to(Frame pointee) {
	        this->data = (this->data & ~0x000ffffffffff000) | (pointee.begin().address() & 0x000ffffffffff000);
	    }
	    flags_t& flags() { return this->data; }
	};*/

	/*template<size_t Level> struct PageTable {
	private:
	    PageTableEntry entries[512];

	public:
	    PageTable() = delete;
	    PageTable()
	        requires(Level > 0 && Level <= 4)
	    {
	        for (auto& entry : this->entries) { entry = PageTableEntry(); }
	    }

	    PageTableEntry& operator[](size_t index) { return this->entries[index]; }

	    std::optional<PageTable<Level - 1> *> get_child_table(size_t index)
	        requires(Level > 1)
	    {
	        if ((*this)[index].flags() & PageTableEntry::PRESENT) {
	            //fprintf(stdserial, "found child table at idx %lli\n", index);
	            return static_cast<PageTable<Level - 1> *>((*this)[index].pointed_frame().begin().to_virtual());
	        }
	        return std::nullopt;
	    }

	    PageTable<Level - 1>& get_or_create_child_table(size_t index, IPhysicalAllocator& page_table_allocator)
	        requires(Level > 1)
	    {
	        auto a = this->get_child_table(index);
	        return **a.or_else([&]() {
	            auto frame = *page_table_allocator.allocate(Frame::size).begin();
	            (*this)[index].point_to(frame);
	            (*this)[index].flags() |= PageTableEntry::PRESENT;
	            (*this)[index].flags() |= PageTableEntry::WRITEABLE;
	            (*this)[index].flags() |= PageTableEntry::USER;
	            fprintf(stdserial,
	                    "new child at idx %llu with flags %c%c%c%c%c\n",
	                    index,
	                    (*this)[index].flags() & PageTableEntry::PRESENT ? 'P' : '-',
	                    (*this)[index].flags() & PageTableEntry::WRITEABLE ? 'W' : 'R',
	                    (*this)[index].flags() & PageTableEntry::USER ? 'U' : 'S',
	                    (*this)[index].flags() & PageTableEntry::GLOBAL ? 'G' : '-',
	                    (*this)[index].flags() & PageTableEntry::NO_EXECUTE ? '-' : 'X');
	            return new (static_cast<PageTable<Level - 1> *>(frame.begin().to_virtual())) PageTable<Level - 1>();
	        });
	    }

	    int print_to(FILE *f, uint64_t addr)
	        requires(Level > 1)
	    {
	        int char_count = 0;
	        for (size_t i = 0; i < 512; ++i) {
	            uint64_t this_addr = addr;
	            this_addr |= i << ((Level - 1) * 9 + 12);
	            if (auto child = this->get_child_table(i)) {
	                char_count += child->print_to(f, this_addr);
	            } else {
	                char_count += fprintf(f, "%lp... -> (nil)\n", this_addr);
	            }
	        }
	        return char_count;
	    }

	    int print_to(FILE *f, uint64_t addr)
	        requires(Level == 1)
	    {
	        int char_count = 0;
	        for (size_t i = 0; i < 512; ++i) {
	            uint64_t this_addr = addr;
	            this_addr |= i << ((Level - 1) * 9 + 12);
	            auto& entry = this->entries[i];
	            if (entry.flags() & PageTableEntry::PRESENT) {
	                char_count += fprintf(f,
	                                      "%lp -> %lp, %c%c%c%c\n",
	                                      this_addr,
	                                      entry.pointed_frame().begin(),
	                                      entry.flags() & PageTableEntry::WRITEABLE ? 'W' : 'R',
	                                      entry.flags() & PageTableEntry::USER ? 'U' : 'S',
	                                      entry.flags() & PageTableEntry::GLOBAL ? 'G' : '-',
	                                      entry.flags() & PageTableEntry::NO_EXECUTE ? '-' : 'X');
	            } else {
	                char_count += fprintf(f, "%lp -> (nil)\n", this_addr);
	            }
	        }
	        return char_count;
	    }
	};*/

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
	protected:
		using data_t = u64;

		PageTableEntryImpl() noexcept : data{0} {}
		explicit PageTableEntryImpl(data_t data) noexcept : data(data) {}
		data_t data;

	public:
		void set_pointed_frame(const frame_t *) noexcept;
		frame_t *pointed_frame() noexcept;
		const frame_t *pointed_frame() const noexcept;
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

		~PageTableEntry() {
			if (auto page_table = this->child_table()) {
				page_table.value()->~PageTable<Level - 1>();
				PageTable<Level - 1>::operator delete(page_table.value(), nullptr);
			}
		}

		PageTableEntry& operator=(const PageTableEntry& rhs) noexcept {
			this->data = rhs.data;
			return *this;
		}

		std::optional<PageTable<Level - 1> *> child_table() noexcept {
			LOG(Log::TRACE, "Request child table - data: %llb", this->data);

			if (static_cast<bool>(this->get_flags() & PageTableFlags::PRESENT)) {
				return static_cast<PageTable<Level - 1> *>(this->pointed_frame()->frame_to_page_map_region().address);
			} else return std::nullopt;
		}
		std::optional<const PageTable<Level - 1> *> child_table() const noexcept {
			LOG(Log::TRACE, "Request child table - data: %llb", this->data);

			if (static_cast<bool>(this->get_flags() & PageTableFlags::PRESENT)) {
				return static_cast<PageTable<Level - 1> *>(this->pointed_frame()->frame_to_page_map_region().address);
			} else return std::nullopt;
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
	};

	template<> class PageTableEntry<1> : public PageTableEntryImpl {
	public:
		//****** NOTE: Don't move these to paging.ipp since the templating stuff breaks ******

		explicit PageTableEntry() noexcept : PageTableEntryImpl{0} {}
		PageTableEntry(const PageTableEntry&) = delete;

		explicit PageTableEntry(const PageTableEntry& other, IPhysicalAllocator& allocator, deep_copy_t);

		PageTableEntry& operator=(const PageTableEntry& rhs) noexcept {
			this->data = rhs.data;
			return *this;
		}
	};

	template<std::size_t Level> class PageTable {
	public:
		using iterator = PageTableEntry<Level>*;
		using const_iterator = const PageTableEntry<Level>*;

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
		frame_t *translate_page(aligned<vaddr_t> page) { __builtin_unreachable(); }

	protected:
		PageTable<4> *l4_table;          //!< Pointer to the level 4 page table for this address space
		IPhysicalAllocator *allocator;   //!< Allocator to use when adding/removing page tables
	};

	class AddressSpace : public AddressSpaceBase {
	public:
		/**
		 * Creates a new, empty address space
		 */
		explicit AddressSpace(IPhysicalAllocator& allocator = allocators.general())
			: AddressSpaceBase(new(&allocator) PageTable<4>, &allocator),
			  ref_count(new atomic_uint_fast64_t(1)) {
			// TODO: copy higher half from KAS into new address space
		}

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
	public:
		KernelAddressSpace(PageTable<4> *l4_table, IPhysicalAllocator& allocator)
			: AddressSpaceBase(l4_table, &allocator) {}
	};

	extern KernelAddressSpace& kas;
	inline KernelAddressSpace& init_kas(IPhysicalAllocator& allocator) {
		new (&kas) KernelAddressSpace{new (&allocator) PageTable<4>{}, allocator};
		return kas;
	}

	/*struct AddressSpace : public PageTable<page_table_root_level> {
	private:
	    std::optional<PhysicalAddress> l4_table;
	    IPhysicalAllocator *page_table_allocator;
	    AddressSpace(PhysicalAddress l4_table, IPhysicalAllocator& page_table_allocator) :
	        l4_table(l4_table),
	        page_table_allocator(&page_table_allocator) {}
	    AddressSpace(PhysicalAddress l4_table) : l4_table(l4_table), page_table_allocator(nullptr) {}
	    AddressSpace() : l4_table(std::nullopt), page_table_allocator(nullptr) {}

	    PageTable<4>& l4_ptr() { return *static_cast<PageTable<4> *>(this->l4_table->to_virtual()); }

	public:
	    AddressSpace(PageTableRoot&) = delete;
	    AddressSpace(PageTableRoot&& other) noexcept : AddressSpace() { std::swap(*this, other); }

	    PageTableRoot& operator=(PageTableRoot&& other) noexcept { std::swap(this->l4_table, other.l4_table); }

	    // TODO: Think about this
	    ~AddressSpace() {
	        fprintf(stdserial, "dropped page table at %lp\n", *this->l4_table);
	        this->page_table_allocator->deallocate({Frame::from_address(this->l4_table)});
	    }

	    static PageTableRoot new_table(IPhysicalAllocator& page_table_allocator);
	    PageTableRoot make_active();
	    Frame translate_page(Page);
	    PhysicalAddress translate_address(VirtualAddress);
	    PageTableEntry& get_entry_for(Page);
	    void map_page_to(Page, Frame, PageTableEntry::flags_t flags, IPhysicalAllocator& page_table_allocator);
	    bool unmap_page(Page page, IPhysicalAllocator& page_table_deallocator);

	    int print_to(FILE *f) { return this->l4_ptr().print_to(f, 0); }

	    static PageTableRoot current_table;
	};*/

	// struct ActiveAddressSpace {};

	// extern AddressSpace current_address_space;

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
