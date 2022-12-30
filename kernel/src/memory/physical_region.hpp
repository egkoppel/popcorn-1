
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_MEMORY_PHYSICAL_REGION_HPP
#define POPCORN_KERNEL_SRC_MEMORY_PHYSICAL_REGION_HPP

#include "types.hpp"

#include <algorithm>
#include <arch/constants.hpp>
#include <cstddef>
#include <main/main.hpp>
#include <popcorn_prelude.h>
#include <utility/iter_wrapper.hpp>
#include <utils.h>

namespace memory {
	class IPhysicalAllocator;

	class PhysicalRegion {
	public:
		using iterator       = iter::iter_wrapper<frame_t *>;
		using const_iterator = iter::iter_wrapper<const frame_t *>;

		PhysicalRegion() noexcept = default;
		PhysicalRegion(usize allocation_size, IPhysicalAllocator& allocator = allocators.general());
		PhysicalRegion(aligned<paddr_t> at,
		               usize allocation_size,
		               IPhysicalAllocator& allocator = allocators.general());
		PhysicalRegion(frame_t *start, usize allocation_size) noexcept :
			start(start),
			allocation_size(allocation_size) {}
		PhysicalRegion(const PhysicalRegion&, shallow_copy_t = shallow_copy) noexcept;
		PhysicalRegion(const PhysicalRegion&, deep_copy_t);
		PhysicalRegion(PhysicalRegion&& other) noexcept : start(other.start), allocation_size(other.allocation_size) {
			other.start = nullptr;
		}
		~PhysicalRegion();

		PhysicalRegion& operator=(const PhysicalRegion&) noexcept;
		PhysicalRegion& operator=(PhysicalRegion&& other) noexcept {
			using std::swap;
			swap(this->start, other.start);
			swap(this->allocation_size, other.allocation_size);
			return *this;
		}

		usize size() const noexcept { return this->allocation_size; }

		iterator begin() noexcept { return iter::iter_wrapper(this->start); }
		iterator end() noexcept {
			return iter::iter_wrapper(&this->start[IDIV_ROUND_UP(this->allocation_size, constants::frame_size)]);
		}
		const_iterator cbegin() const noexcept { return iter::iter_wrapper(const_cast<const frame_t *>(this->start)); }
		const_iterator cend() const noexcept {
			return iter::iter_wrapper(const_cast<const frame_t *>(
					&this->start[IDIV_ROUND_UP(this->allocation_size, constants::frame_size)]));
		}
		const_iterator begin() const noexcept { return this->cbegin(); }
		const_iterator end() const noexcept { return this->cend(); }

		frame_t *release() noexcept;

	private:
		frame_t *start        = nullptr;
		usize allocation_size = 0;

		void decrement_and_drop() noexcept;
	};
}   // namespace memory

#endif   //POPCORN_KERNEL_SRC_MEMORY_PHYSICAL_REGION_HPP
