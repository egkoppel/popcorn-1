/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_MONOTONIC_ALLOCATOR_HPP
#define HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_MONOTONIC_ALLOCATOR_HPP

#include "../physical_allocator.hpp"
#include "../types.hpp"

#include <multiboot/memory_map.hpp>
#include <stdint.h>

namespace memory::physical_allocators {
	/**
	 * Add stuff
	 */
	class MonotonicAllocator : public IPhysicalAllocator {
	private:
		aligned<paddr_t> next_frame;
		aligned<paddr_t> end_frame;
		aligned<paddr_t> kernel_start_frame;
		aligned<paddr_t> kernel_end_frame;
		aligned<paddr_t> multiboot_start_frame;
		aligned<paddr_t> multiboot_end_frame;
		multiboot::tags::MemoryMap *mem_map;

	public:
		MonotonicAllocator(paddr_t start_addr,
		                   paddr_t end_addr,
		                   paddr_t kernel_start,
		                   paddr_t kernel_end,
		                   paddr_t multiboot_start,
		                   paddr_t multiboot_end,
		                   multiboot::tags::MemoryMap *mem_map) noexcept :
			next_frame(aligned<paddr_t>::aligned_up(start_addr)),
			end_frame(aligned<paddr_t>::aligned_up(end_addr)),
			kernel_start_frame(aligned<paddr_t>::aligned_down(kernel_start)),
			kernel_end_frame(aligned<paddr_t>::aligned_up(kernel_end)),
			multiboot_start_frame(aligned<paddr_t>::aligned_down(multiboot_start)),
			multiboot_end_frame(aligned<paddr_t>::aligned_up(multiboot_end)),
			mem_map(mem_map) {}
		MonotonicAllocator(const MonotonicAllocator&)             = delete;
		MonotonicAllocator(MonotonicAllocator&&) noexcept   = default;
		MonotonicAllocator& operator=(const MonotonicAllocator&)  = delete;
		MonotonicAllocator& operator=(MonotonicAllocator&&) = default;

		frame_t *allocate_(u64 byte_length) override;
		void deallocate_(const frame_t *frames, u64) noexcept override {}

		aligned<paddr_t> get_next_frame() const { return this->next_frame; }
		aligned<paddr_t> get_kernel_start_frame() const { return this->kernel_start_frame; }
		aligned<paddr_t> get_kernel_end_frame() const { return this->kernel_end_frame; }
		aligned<paddr_t> get_multiboot_start_frame() const { return this->multiboot_start_frame; }
		aligned<paddr_t> get_multiboot_end_frame() const { return this->multiboot_end_frame; }
		multiboot::tags::MemoryMap *get_mem_map() const { return this->mem_map; }

	protected:
		const char *name() noexcept override {return "MonotonicAllocator"; }
	};
}   // namespace memory::physical_allocators

#endif   // HUGOS_KERNEL_SRC_MEMORY_PHYSICAL_ALLOCATORS_MONOTONIC_ALLOCATOR
