/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MEMORY_HPP
#define HUGOS_MEMORY_HPP

#include <arch/constants.hpp>
#include <assert.h>
#include <bit>
#include <compare>
#include <initializer_list>
#include <mixin.hpp>
#include <new>
#include <popcorn_prelude.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace memory {
	struct vaddr_t;
	struct paddr_t;
	class IPhysicalAllocator;
	template<class, std::size_t = constants::frame_size> struct aligned;

	struct frame_t {
		usize ref_count                  = 0;
		IPhysicalAllocator *allocated_by = nullptr;

		usize number() const;
		aligned<vaddr_t> frame_to_page_map_region() const;

		frame_t(const frame_t&) = delete;
	};

	inline frame_t *const mem_map = reinterpret_cast<frame_t *>(constants::mem_map_start);

	struct vaddr_t : mixin::unary_post_ops<vaddr_t>,
					 mixin::binary_ops<vaddr_t, usize> {
		using difference_type = usize;

		usize address;

		constexpr vaddr_t& operator+=(usize rhs) noexcept {
			this->address += rhs;
			return *this;
		}
		constexpr vaddr_t& operator-=(usize rhs) noexcept {
			this->address -= rhs;
			return *this;
		}
		constexpr vaddr_t& operator++() noexcept {
			this->address++;
			return *this;
		}
		constexpr vaddr_t& operator--() noexcept {
			this->address--;
			return *this;
		}
		constexpr difference_type operator-(vaddr_t rhs) const noexcept { return this->address - rhs.address; }

		using mixin::unary_post_ops<vaddr_t>::operator++;
		using mixin::unary_post_ops<vaddr_t>::operator--;
		using mixin::binary_ops<vaddr_t, usize>::operator+;
		using mixin::binary_ops<vaddr_t, usize>::operator-;

		template<usize Level> constexpr usize page_table_index() const noexcept;
		template<> constexpr usize page_table_index<4>() const noexcept { return (this->address >> 39) & 0777; }
		template<> constexpr usize page_table_index<3>() const noexcept { return (this->address >> 30) & 0777; }
		template<> constexpr usize page_table_index<2>() const noexcept { return (this->address >> 21) & 0777; }
		template<> constexpr usize page_table_index<1>() const noexcept { return (this->address >> 12) & 0777; }

		template<class T> explicit operator T *() const noexcept { return reinterpret_cast<T *>(this->address); }

		constexpr paddr_t devirtualise() const noexcept;
	};

	constexpr bool operator!=(vaddr_t lhs, vaddr_t rhs) noexcept { return lhs.address != rhs.address; }
	constexpr bool operator==(vaddr_t lhs, vaddr_t rhs) noexcept { return (lhs.address <=> rhs.address) == 0; }
	constexpr std::strong_ordering operator<=>(vaddr_t lhs, vaddr_t rhs) noexcept {
		return lhs.address <=> rhs.address;
	}

	struct paddr_t : mixin::unary_post_ops<paddr_t>,
					 mixin::binary_ops<paddr_t, usize> {
		using difference_type = usize;
		usize address;

		constexpr paddr_t& operator+=(usize rhs) noexcept {
			this->address += rhs;
			return *this;
		}
		constexpr paddr_t& operator-=(usize rhs) noexcept {
			this->address -= rhs;
			return *this;
		}
		constexpr paddr_t& operator++() noexcept {
			this->address++;
			return *this;
		}
		constexpr paddr_t& operator--() noexcept {
			this->address--;
			return *this;
		}
		constexpr difference_type operator-(vaddr_t rhs) const noexcept { return this->address - rhs.address; }

		using mixin::unary_post_ops<paddr_t>::operator++;
		using mixin::unary_post_ops<paddr_t>::operator--;
		using mixin::binary_ops<paddr_t, usize>::operator+;
		using mixin::binary_ops<paddr_t, usize>::operator-;

		constexpr vaddr_t virtualise() const noexcept {
			return {.address = this->address + constants::page_offset_start};
		}
	};
	constexpr bool operator!=(paddr_t lhs, paddr_t rhs) noexcept { return lhs.address != rhs.address; }
	constexpr bool operator==(paddr_t lhs, paddr_t rhs) noexcept { return (lhs.address <=> rhs.address) == 0; }
	constexpr std::strong_ordering operator<=>(paddr_t lhs, paddr_t rhs) noexcept {
		return lhs.address <=> rhs.address;
	}

	constexpr paddr_t vaddr_t::devirtualise() const noexcept {
		return {.address = this->address - constants::page_offset_start};
	}

	struct paddr32_t {
		u32 address;
		constexpr explicit(false) operator paddr_t() const noexcept { return {.address = this->address}; }
	};

	struct vaddr32_t {
		u32 address;
		constexpr explicit(false) operator vaddr_t() const noexcept { return {.address = this->address}; }
	};

	template<class T, std::size_t alignment> struct aligned;

	template<std::size_t alignment>
	struct aligned<vaddr_t, alignment> : public mixin::unary_post_ops<aligned<vaddr_t, alignment>>,
										 public mixin::binary_ops<aligned<vaddr_t, alignment>, usize> {
		constexpr explicit(false) aligned(vaddr_t addr) {
#ifndef NDEBUG
			if ((addr.address & (alignment - 1)) != 0) throw std::runtime_error("Misaligned address");
#endif
			this->address = addr;
		}

		static aligned<vaddr_t, alignment> aligned_down(vaddr_t addr) {
			return vaddr_t{.address = addr.address & ~(alignment - 1)};
		}

		static aligned<vaddr_t, alignment> aligned_up(vaddr_t addr) { return aligned_down(addr + alignment - 1); }

		constexpr explicit(false) operator vaddr_t() { return this->address; }
		constexpr aligned& operator++() {
			this->address += alignment;
			return *this;
		}
		constexpr aligned& operator--() {
			this->address -= alignment;
			return *this;
		}
		constexpr aligned& operator+=(usize rhs) {
			this->address += alignment * rhs;
			return *this;
		}
		constexpr aligned& operator-=(usize rhs) {
			this->address -= alignment * rhs;
			return *this;
		}
		constexpr bool operator!=(aligned rhs) { return this->address != rhs.address; }

		using mixin::unary_post_ops<aligned>::operator++;
		using mixin::unary_post_ops<aligned>::operator--;
		using mixin::binary_ops<aligned, usize>::operator+;
		using mixin::binary_ops<aligned, usize>::operator-;

		frame_t *page_map_region_to_frame()
			requires(alignment == constants::frame_size)
		{
#ifndef NDEBUG
			if (this->address.address < constants::page_offset_start
			    || this->address.address >= constants::page_offset_end)
				throw std::runtime_error("vaddr_t outside of page map region - attempted conversion to frame_t*");
#endif
			return &mem_map[this->address.address / constants::frame_size];
		}

		const frame_t *page_map_region_to_frame() const
			requires(alignment == constants::frame_size)
		{
#ifndef NDEBUG
			if (this->address.address < constants::page_offset_start
			    || this->address.address >= constants::page_offset_end)
				throw std::runtime_error("vaddr_t outside of page map region - attempted conversion to frame_t*");
#endif
			return &mem_map[this->address.address / constants::frame_size];
		}

		vaddr_t address;
	};
	template<std::size_t alignment>
	struct aligned<paddr_t, alignment> : public mixin::unary_post_ops<aligned<paddr_t, alignment>>,
										 public mixin::binary_ops<aligned<paddr_t, alignment>, usize> {
		constexpr explicit(false) aligned(paddr_t addr) {
#ifndef NDEBUG
			if ((addr.address & (alignment - 1)) != 0) throw std::runtime_error("Misaligned address");
#endif
			this->address = addr;
		}

		static aligned<paddr_t, alignment> aligned_down(paddr_t addr) {
			return paddr_t{.address = addr.address & ~(alignment - 1)};
		}

		static aligned<paddr_t, alignment> aligned_up(paddr_t addr) { return aligned_down(addr + alignment - 1); }

		constexpr explicit(false) operator paddr_t() { return this->address; }

		constexpr aligned& operator++() {
			this->address += alignment;
			return *this;
		}
		constexpr aligned& operator--() {
			this->address -= alignment;
			return *this;
		}
		constexpr aligned& operator+=(usize rhs) {
			this->address += alignment * rhs;
			return *this;
		}
		constexpr aligned& operator-=(usize rhs) {
			this->address -= alignment * rhs;
			return *this;
		}
		constexpr bool operator!=(aligned rhs) { return this->address != rhs.address; }

		using mixin::unary_post_ops<aligned>::operator++;
		using mixin::unary_post_ops<aligned>::operator--;
		using mixin::binary_ops<aligned, usize>::operator+;
		using mixin::binary_ops<aligned, usize>::operator-;

		constexpr frame_t *frame() noexcept
			requires(alignment == constants::frame_size)
		{
			return &mem_map[this->address.address / constants::frame_size];
		}

		constexpr const frame_t *frame() const noexcept
			requires(alignment == constants::frame_size)
		{
			return &mem_map[this->address / constants::frame_size];
		}

		paddr_t address;
	};

	namespace literals {
		consteval paddr_t operator""_pa(unsigned long long addr) { return {.address = addr}; }
		consteval vaddr_t operator""_va(unsigned long long addr) { return {.address = addr}; }
		consteval aligned<paddr_t> operator""_palign(unsigned long long addr) {
			if ((addr & ~07777) != addr) throw;
			return paddr_t{.address = addr};
		}
		consteval aligned<vaddr_t> operator""_valign(unsigned long long addr) {
			if ((addr & ~07777) != addr) throw;
			return vaddr_t{.address = addr};
		}
	}   // namespace literals

	/*class Frame {
	private:
		PhysicalAddress start_address;
		explicit constexpr Frame(PhysicalAddress a) : start_address(a) {}

	public:
		class misaligned_frame_exception : public std::exception {
		public:
			const char *what() const noexcept override { return "misaligned_frame_exception"; }
		};

		static constexpr uint64_t size = 0x1000;

		// Constructors
		constexpr static Frame from_address(PhysicalAddress a) {
			if (!((a.address() & ~(Frame::size - 1)) == a.address())) throw misaligned_frame_exception();
			return Frame(a);
		}
		constexpr static Frame containing_address(PhysicalAddress a) {
			return Frame(PhysicalAddress((a.address() / Frame::size) * Frame::size));
		}

		// End getters
		constexpr inline PhysicalAddress begin() const { return this->start_address; }
		constexpr inline PhysicalAddress end() const { return this->start_address + Frame::size; }

		constexpr inline uint64_t number() const { return this->start_address.address() / Frame::size; }

		// Comparison
		bool operator!=(const Frame& rhs) const                  = default;
		std::strong_ordering operator<=>(const Frame& rhs) const = default;

		// Increment/decrement by frame size
		Frame operator+(const uint64_t rhs) const { return Frame(this->start_address + rhs * Frame::size); }
		Frame operator-(const uint64_t rhs) const { return Frame(this->start_address - rhs * Frame::size); }
		Frame& operator++() {
			this->start_address += Frame::size;
			return *this;
		}
		Frame operator++(int) {
			auto r = *this;
			this->start_address += Frame::size;
			return r;
		}
		Frame& operator--() {
			this->start_address -= Frame::size;
			return *this;
		}
		Frame operator--(int) {
			auto r = *this;
			this->start_address -= Frame::size;
			return r;
		}
	};

	class Page {
	private:
		VirtualAddress start_address;
		explicit constexpr Page(VirtualAddress a) : start_address(a) {}

	public:
		class misaligned_page_exception : public std::exception {
		public:
			const char *what() const noexcept override { return "misaligned_page_exception"; }
		};

		static constexpr uint64_t size = 0x1000;

		// Constructors
		constexpr static Page from_address(VirtualAddress a) {
			if (!((a.address() & ~(Page::size - 1)) == a.address())) throw misaligned_page_exception();
			return Page(a);
		}
		constexpr static Page containing_address(VirtualAddress a) { return Page(a.aligned_down(Page::size)); }

		// End getters
		constexpr inline VirtualAddress begin() const { return this->start_address; }
		constexpr inline VirtualAddress end() const { return this->start_address + Page::size; }

		constexpr inline uint64_t number() const { return this->start_address.address() / Page::size; }

		// Comparison
		bool operator!=(const Page& rhs) const                  = default;
		std::strong_ordering operator<=>(const Page& rhs) const = default;

		// Increment/decrement by frame size
		Page operator+(const uint64_t rhs) const { return Page(this->start_address + rhs * Page::size); }
		Page operator-(const uint64_t rhs) const { return Page(this->start_address - rhs * Page::size); }
		Page& operator++() {
			this->start_address += Page::size;
			return *this;
		}
		Page operator++(int) {
			auto r = *this;
			this->start_address += Page::size;
			return r;
		}
		Page& operator--() {
			this->start_address -= Page::size;
			return *this;
		}
		Page operator--(int) {
			auto r = *this;
			this->start_address -= Page::size;
			return r;
		}
	};

	class FrameRange {
	public:
		class iterator {
		private:
			Frame current;

		public:
			explicit iterator(Frame current) : current(current) {}
			Frame& operator*() { return this->current; }
			Frame *operator->() { return &this->current; }
			iterator& operator++() {
				this->current++;
				return *this;
			}
			bool operator==(const iterator& rhs) const { return current == rhs.current; }
			bool operator!=(const iterator& rhs) const { return !(rhs == *this); }
		};

	private:
		Frame start;
		Frame end_;

	public:
		explicit(false) FrameRange(Frame frame) : start(frame), end_(frame + 1) {}
		FrameRange(Frame start, Frame end) : start(start), end_(end) {}

		iterator begin() const { return iterator(this->start); }
		iterator end() const { return iterator(this->end_); }
	};

	static_assert(sizeof(FrameRange) == 2 * sizeof(char *));

	class FrameVector {
	public:
		class iterator {
		private:
			FrameRange::iterator current_range_iter;
			size_t vec_index;
			const FrameVector& parent;

		public:
			bool operator==(const iterator& rhs) const {
				return (this->current_range_iter == rhs.current_range_iter) && (&this->parent == &rhs.parent);
			}
			bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
			Frame& operator*() { return *this->current_range_iter; }
			FrameRange::iterator operator->() { return this->current_range_iter; }
			iterator& operator++() {
				++this->current_range_iter;
				if (!parent.is_one() && (this->current_range_iter == (*parent.data.list.dat)[vec_index].end())) {
					if (++vec_index == parent.data.list.dat->size())
						this->current_range_iter = (*parent.data.list.dat)[vec_index - 1].end();
					else this->current_range_iter = (*parent.data.list.dat)[vec_index].begin();
				}
				return *this;
			}

			iterator(const FrameRange::iterator& current_range_iter, size_t vec_index, const FrameVector& parent) :
				current_range_iter(current_range_iter),
				vec_index(vec_index),
				parent(parent) {}
		};

	private:
		union {
			FrameRange single;
			struct {
				std::vector<FrameRange> *dat;
				void *unused;
			} list;
		} data{
				.list = {nullptr, nullptr}
        };

		template<class InputIt>
		explicit FrameVector(InputIt begin, InputIt end) :
			data{
					.list = {.dat = new std::vector<FrameRange>(begin, end), .unused = nullptr}
        } {}

		[[nodiscard]] bool is_one() const { return this->data.list.unused; }

	public:
		explicit(false) FrameVector(const FrameRange range) : data{.single = range} {}
		FrameVector(std::initializer_list<FrameRange> ilist) :
			FrameVector(ilist.size() == 1 ? FrameVector(*ilist.begin()) : FrameVector(ilist.begin(), ilist.end())) {}
		~FrameVector() {
			if (!this->is_one()) delete this->data.list.dat;
			else this->data.single.~FrameRange();
		}
		FrameVector(FrameVector&) = delete;
		FrameVector(FrameVector&& other) noexcept {
			if (!other.is_one()) this->data.list.dat = new std::vector<FrameRange>(std::move(*other.data.list.dat));
			else { this->data.single = other.data.single; }
		}
		iterator begin() const {
			if (this->is_one()) return {this->data.single.begin(), 0, *this};
			else if (!this->data.list.dat->empty()) return {this->data.list.dat->front().begin(), 0, *this};
			else return {this->data.single.begin(), 0, *this};
		}
		iterator end() const {
			if (this->is_one()) return {this->data.single.end(), 0, *this};
			else if (!this->data.list.dat->empty()) return {this->data.list.dat->back().end(), 0, *this};
			else return this->begin();
		}
		size_t byte_length() {
			size_t ret = 0;
			for (auto& frame : *this) { ret += Frame::size; }
			return ret;
		}
	};
	static_assert(sizeof(FrameVector) == sizeof(char *) * 2);*/

	void init_sbrk();
	//extern "C" void *sbrk(intptr_t increment);
}   // namespace memory

using namespace memory::literals;

#endif   // HUGOS_MEMORY_HPP
