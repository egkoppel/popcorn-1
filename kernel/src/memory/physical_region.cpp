/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "physical_region.hpp"

#include "physical_allocator.hpp"

namespace memory {
	PhysicalRegion::PhysicalRegion(usize allocation_size, IPhysicalAllocator& allocator)
		: start(allocator.allocate(allocation_size)),
		  allocation_size(allocation_size) {
		this->start->ref_count = 1;
	}

	PhysicalRegion::PhysicalRegion(aligned<paddr_t> at, usize allocation_size, IPhysicalAllocator& allocator)
		: start(allocator.allocate(at, allocation_size)),
		  allocation_size(allocation_size) {
		this->start->ref_count = 1;
	}

	PhysicalRegion::~PhysicalRegion() {
		this->decrement_and_drop();
	}

	PhysicalRegion::PhysicalRegion(const PhysicalRegion& rhs, shallow_copy_t) noexcept
		: start(rhs.start),
		  allocation_size(rhs.allocation_size) {
		this->start->ref_count++;
	}

	PhysicalRegion& PhysicalRegion::operator=(const PhysicalRegion& rhs) noexcept {
		this->decrement_and_drop();
		this->start           = rhs.start;
		this->allocation_size = rhs.allocation_size;
		this->start->ref_count++;
		return *this;
	}

	void PhysicalRegion::decrement_and_drop() noexcept {
		if (!this->start) return;
		if (--this->start->ref_count == 0) IPhysicalAllocator::deallocate(this->start, this->allocation_size);
	}

	frame_t *PhysicalRegion::release() noexcept {
		auto ret    = this->start;
		this->start = nullptr;
		return ret;
	}

	usize frame_t::number() const {
		return (this - mem_map);
	}

	aligned<vaddr_t> frame_t::frame_to_page_map_region() const {
		return vaddr_t{.address = this->addr() + constants::page_offset_start};
	}
}   // namespace memory
