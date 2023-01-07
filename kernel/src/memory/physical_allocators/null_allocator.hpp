
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_NULL_ALLOCATOR_HPP
#define POPCORN_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_NULL_ALLOCATOR_HPP

#include <memory/physical_allocator.hpp>

namespace memory::physical_allocators {
	class NullAllocator : public IPhysicalAllocator {
	protected:
		frame_t *allocate_(u64) override { THROW(std::bad_alloc()); }
		frame_t *allocate_at_(aligned<paddr_t> start, u64) override { return start.frame(); }
		void deallocate_(const frame_t *frame, u64) noexcept override {}
	};
}   // namespace memory::physical_allocators

#endif   // POPCORN_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_NULL_ALLOCATOR_HPP
