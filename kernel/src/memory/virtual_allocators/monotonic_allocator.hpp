
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_MEMORY_VIRTUAL_ALLOCATORS_MONOTONIC_ALLOCATOR_HPP
#define HUGOS_KERNEL_SRC_MEMORY_VIRTUAL_ALLOCATORS_MONOTONIC_ALLOCATOR_HPP

#include "../types.hpp"
#include "../virtual_allocator.hpp"

#include <algorithm>
#include <stdint.h>

namespace memory::virtual_allocators {
	class MonotonicAllocator : public IVirtualAllocator {
	private:
		aligned<vaddr_t> current_;
		aligned<vaddr_t> end_;

	public:
		MonotonicAllocator(aligned<vaddr_t> start, aligned<vaddr_t> end) : current_(start), end_(end) {}
		MonotonicAllocator(MonotonicAllocator&)                 = delete;
		MonotonicAllocator(MonotonicAllocator&& other) noexcept = default;
		MonotonicAllocator& operator=(MonotonicAllocator&)      = delete;
		MonotonicAllocator& operator=(MonotonicAllocator&&)     = default;

		aligned<vaddr_t> allocate_(uint64_t byte_length) override;
		void deallocate_(aligned<vaddr_t> start, uint64_t byte_length) noexcept override {}
	};
}   // namespace memory::virtual_allocators
#endif   //HUGOS_KERNEL_SRC_MEMORY_VIRTUAL_ALLOCATORS_MONOTONIC_ALLOCATOR_HPP
