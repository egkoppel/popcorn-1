
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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
#include <utility/step_by.hpp>

namespace memory {
	class IVirtualAllocator;

	struct general_allocator_t {
		static aligned<vaddr_t> allocate(u64 size);
		static void deallocate(aligned<vaddr_t>, u64 size) noexcept;
	};
	inline constexpr general_allocator_t general_allocator{};

	template<class Allocator = general_allocator_t> class VirtualRegion {
	public:
		explicit VirtualRegion(Allocator allocator = Allocator()) : start{{.address = 0}}, allocation_page_count{0} {}
		explicit VirtualRegion(usize allocation_size, Allocator allocator = Allocator());
		VirtualRegion(const VirtualRegion&) noexcept = delete;
		explicit VirtualRegion(VirtualRegion&& other) noexcept : VirtualRegion() { std::swap(*this, other); }
		~VirtualRegion() { throw "Unimplemented"; }

		VirtualRegion& operator=(const VirtualRegion&) noexcept = delete;
		VirtualRegion& operator=(VirtualRegion&& other) noexcept { std::swap(*this, other); }

		auto begin() const { return this->start; }
		auto end() const { return this->start + this->allocation_page_count; }

	private:
		aligned<vaddr_t> start;
		usize allocation_page_count;
	};
}   // namespace memory

#endif   //POPCORN_KERNEL_SRC_MEMORY_VIRTUAL_REGION_HPP
