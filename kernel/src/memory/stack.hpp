/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_STACK_HPP
#define HUGOS_STACK_HPP

#include "physical_region.hpp"
#include "virtual_region.hpp"

#include <concepts>

namespace memory {
	/**
	 * A kernel use stack with a guard page
	 * @tparam VAllocator The virtual allocator to use
	 */
	template<class VAllocator = general_allocator_t> class KStack {
	public:
		explicit KStack(VAllocator&& vallocator = VAllocator()) :
			virtual_region{std::forward<VAllocator>(vallocator)} {};
		explicit KStack(usize stack_size,
		                IPhysicalAllocator& pallocator = allocators.general(),
		                VAllocator&& vallocator        = VAllocator());
		KStack(const KStack&) = delete;
		KStack(KStack&& other) noexcept :
			backing_region(std::move(other.backing_region)),
			virtual_region(std::move(other.virtual_region)) {
			other.backing_region = PhysicalRegion{};
			other.virtual_region = VirtualRegion<VAllocator>{};
		}
		~KStack();

		KStack& operator=(const KStack&) = delete;
		KStack& operator=(KStack&& other) noexcept {
			using std::swap;
			swap(this->backing_region, other.backing_region);
			swap(this->virtual_region, other.virtual_region);
			return *this;
		}

		usize size() const noexcept { return this->backing_region.size(); }

		using iterator       = typename VirtualRegion<VAllocator>::iterator;
		using const_iterator = typename VirtualRegion<VAllocator>::const_iterator;

		iterator top() noexcept { return this->virtual_region.end(); }
		iterator bottom() noexcept { return this->virtual_region.begin(); }
		const_iterator top() const noexcept { return this->virtual_region.cend(); }
		const_iterator bottom() const noexcept { return this->virtual_region.cbegin(); }

	private:
		PhysicalRegion backing_region{};
		VirtualRegion<VAllocator> virtual_region;
	};
}   // namespace memory

#include "stack.ipp"

#endif   // HUGOS_STACK_HPP