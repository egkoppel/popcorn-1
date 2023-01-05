
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_MEMORY_VIRTUAL_REGION_HPP
#define POPCORN_KERNEL_SRC_MEMORY_VIRTUAL_REGION_HPP

#include "types.hpp"

#include <algorithm>
#include <main/main.hpp>
#include <popcorn_prelude.h>
#include <utility/iter_wrapper.hpp>
#include <utility/step_by.hpp>
#include <utils.h>

namespace memory {
	class IVirtualAllocator;

	struct general_allocator_t {
		static aligned<vaddr_t> allocate(u64 size);
		static void deallocate(aligned<vaddr_t>, u64 size) noexcept;
	};
	inline constexpr general_allocator_t general_allocator{};

	template<class Allocator = general_allocator_t> class VirtualRegion {
	public:
		using iterator       = iter::iter_wrapper<aligned<vaddr_t>>;
		using const_iterator = iter::iter_wrapper<aligned<vaddr_t>>;

		explicit VirtualRegion(Allocator allocator = Allocator()) :
			start{{.address = 0}},
			allocation_size{0},
			allocator{allocator} {}
		explicit VirtualRegion(usize allocation_size, Allocator allocator = Allocator()) :
			start{allocator.allocate(allocation_size)},
			allocation_size{allocation_size},
			allocator{allocator} {}
		explicit VirtualRegion(aligned<vaddr_t> start, usize allocation_size, Allocator allocator = Allocator()) :
			start{start},
			allocation_size{allocation_size},
			allocator{allocator} {}
		VirtualRegion(const VirtualRegion&) noexcept = delete;
		explicit VirtualRegion(VirtualRegion&& other) noexcept :
			start(other.start),
			allocation_size(other.allocation_size) {
			other.allocation_size = 0;
		}
		~VirtualRegion() {
			if (this->allocation_size != 0) { this->allocator.deallocate(this->start, this->allocation_size); }
		}

		VirtualRegion& operator=(const VirtualRegion&) noexcept = delete;
		VirtualRegion& operator=(VirtualRegion&& other) noexcept {
			using std::swap;
			swap(this->start, other.start);
			swap(this->allocation_size, other.allocation_size);
			return *this;
		}

		iterator begin() { return iter::iter_wrapper(this->start); }
		iterator end() {
			return iter::iter_wrapper(this->start + IDIV_ROUND_UP(allocation_size, constants::frame_size));
		}
		const_iterator cbegin() const { return iter::iter_wrapper(this->start); }
		const_iterator cend() const {
			return iter::iter_wrapper(this->start + IDIV_ROUND_UP(allocation_size, constants::frame_size));
		}
		const_iterator begin() const { return this->cbegin(); }
		const_iterator end() const { return this->cend(); }

		std::size_t size() { return this->allocation_size; }

	private:
		aligned<vaddr_t> start;
		usize allocation_size;
		[[no_unique_address]] Allocator allocator;
	};
}   // namespace memory

#endif   //POPCORN_KERNEL_SRC_MEMORY_VIRTUAL_REGION_HPP
