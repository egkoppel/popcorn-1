/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
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
		usize addr() const { return this->number() * constants::frame_size; }
		aligned<vaddr_t> frame_to_page_map_region() const;

		frame_t(const frame_t&) = delete;
	};

	// Starts at the 1M mark
	inline frame_t *const mem_map = reinterpret_cast<frame_t *>(constants::mem_map_start) - 256;

	struct [[gnu::packed]] vaddr_t : mixin::unary_post_ops<vaddr_t>,
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

	constexpr bool operator!=(vaddr_t lhs, vaddr_t rhs) noexcept {
		return lhs.address != rhs.address;
	}
	constexpr bool operator==(vaddr_t lhs, vaddr_t rhs) noexcept {
		return (lhs.address <=> rhs.address) == 0;
	}
	constexpr std::strong_ordering operator<=>(vaddr_t lhs, vaddr_t rhs) noexcept {
		return lhs.address <=> rhs.address;
	}

	struct [[gnu::packed]] paddr_t : mixin::unary_post_ops<paddr_t>,
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
	constexpr bool operator!=(paddr_t lhs, paddr_t rhs) noexcept {
		return lhs.address != rhs.address;
	}
	constexpr bool operator==(paddr_t lhs, paddr_t rhs) noexcept {
		return (lhs.address <=> rhs.address) == 0;
	}
	constexpr std::strong_ordering operator<=>(paddr_t lhs, paddr_t rhs) noexcept {
		return lhs.address <=> rhs.address;
	}

	constexpr paddr_t vaddr_t::devirtualise() const noexcept {
		return {.address = this->address - constants::page_offset_start};
	}

	struct [[gnu::packed]] paddr32_t {
		u32 address;
		constexpr explicit(false) operator paddr_t() const noexcept { return {.address = this->address}; }
	};

	struct [[gnu::packed]] vaddr32_t {
		u32 address;
		constexpr explicit(false) operator vaddr_t() const noexcept { return {.address = this->address}; }
	};

	template<class T, std::size_t alignment> struct aligned;

	template<std::size_t alignment>
	struct aligned<vaddr_t, alignment> : public mixin::unary_post_ops<aligned<vaddr_t, alignment>>,
										 public mixin::binary_ops<aligned<vaddr_t, alignment>, usize> {
		constexpr explicit(false) aligned(vaddr_t addr) {
#ifndef NDEBUG
			if ((addr.address & (alignment - 1)) != 0) THROW(std::runtime_error("Misaligned address"));
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

		frame_t *page_map_region_to_frame() requires(alignment == constants::frame_size)
		{
#ifndef NDEBUG
			if (this->address.address < constants::page_offset_start
			    || this->address.address >= constants::page_offset_end)
				THROW(std::runtime_error("vaddr_t outside of page map region - attempted conversion to frame_t*"));
#endif
			return &mem_map[(this->address.address - constants::page_offset_start) / constants::frame_size];
		}

		const frame_t *page_map_region_to_frame() const requires(alignment == constants::frame_size)
		{
#ifndef NDEBUG
			if (this->address.address < constants::page_offset_start
			    || this->address.address >= constants::page_offset_end)
				THROW(std::runtime_error("vaddr_t outside of page map region - attempted conversion to frame_t*"));
#endif
			return &mem_map[(this->address.address - constants::page_offset_start) / constants::frame_size];
		}

		vaddr_t address;
	};
	template<std::size_t alignment>
	struct aligned<paddr_t, alignment> : public mixin::unary_post_ops<aligned<paddr_t, alignment>>,
										 public mixin::binary_ops<aligned<paddr_t, alignment>, usize> {
		constexpr explicit(false) aligned(paddr_t addr) {
#ifndef NDEBUG
			if ((addr.address & (alignment - 1)) != 0) THROW(std::runtime_error("Misaligned address"));
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

		constexpr frame_t *frame() noexcept requires(alignment == constants::frame_size)
		{
			return mem_map + (this->address.address / constants::frame_size);
		}

		constexpr const frame_t *frame() const noexcept requires(alignment == constants::frame_size)
		{
			return mem_map + (this->address.address / constants::frame_size);
		}

		paddr_t address;
	};

	namespace literals {
		consteval paddr_t operator""_pa(unsigned long long addr) {
			return {.address = addr};
		}
		consteval vaddr_t operator""_va(unsigned long long addr) {
			return {.address = addr};
		}
		consteval aligned<paddr_t> operator""_palign(unsigned long long addr) {
			if ((addr & ~07777) != addr) throw;
			return paddr_t{.address = addr};
		}
		consteval aligned<vaddr_t> operator""_valign(unsigned long long addr) {
			if ((addr & ~07777) != addr) throw;
			return vaddr_t{.address = addr};
		}
	}   // namespace literals


	void init_sbrk();
	// extern "C" void *sbrk(intptr_t increment);
}   // namespace memory

using namespace memory::literals;

#endif   // HUGOS_MEMORY_HPP
