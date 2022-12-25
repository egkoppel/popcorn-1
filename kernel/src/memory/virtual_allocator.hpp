
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_VIRTUAL_ALLOCATOR_HPP
#define HUGOS_VIRTUAL_ALLOCATOR_HPP

#include "types.hpp"

#include <utils.h>

namespace memory {
	class IVirtualAllocator {
	public:
		/**
		 * Allocate enough pages to contain at least \p byte_length
		 * @param byte_length The length of the allocation
		 * @return The start of the returned allocation
		 * @throws std::bad_alloc Failed to allocate memory
		 */
		aligned<vaddr_t> allocate(uint64_t byte_length);
		void deallocate(aligned<vaddr_t> start, uint64_t byte_length) noexcept;

	protected:
		virtual aligned<vaddr_t> allocate_(uint64_t byte_length)                        = 0;
		virtual void deallocate_(aligned<vaddr_t> start, uint64_t byte_length) noexcept = 0;
	};
}   // namespace memory

#endif   //HUGOS_VIRTUAL_ALLOCATOR_HPP
