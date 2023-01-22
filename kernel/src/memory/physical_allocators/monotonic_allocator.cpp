/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "monotonic_allocator.hpp"

#include <cstdio>
#include <utils.h>

namespace memory::physical_allocators {
	frame_t *MonotonicAllocator::allocate_(u64 byte_length) {
		while (true) {
			auto allocation_end_frame = this->next_frame + IDIV_ROUND_UP(byte_length, constants::frame_size);

			// fprintf(stdserial, "Attempt alloc at %p\n", attempt);
			LOG(Log::TRACE, "Attempt alloc at %p", this->next_frame);
			if ((this->next_frame >= this->kernel_start_frame && this->next_frame < this->kernel_end_frame)
			    || (this->next_frame >= allocation_end_frame && allocation_end_frame < this->kernel_end_frame)) {
				// fprintf(stdserial, "bump alloc kernel jump\n");
				LOG(Log::TRACE, "bump alloc kernel jump");
				this->next_frame = this->kernel_end_frame;
				continue;
			}

			if ((this->next_frame >= this->multiboot_start_frame && this->next_frame < this->multiboot_end_frame)
			    || (this->next_frame >= allocation_end_frame && allocation_end_frame < this->multiboot_end_frame)) {
				LOG(Log::TRACE, "bump alloc multiboot jump");
				this->next_frame = this->multiboot_end_frame;
				continue;
			}

			for (auto entry : *this->mem_map) {
				LOG(Log::TRACE, "check in region %p -> %p", entry.get_start_address(), entry.get_end_address());
				if (entry.get_start_address() <= this->next_frame && allocation_end_frame < entry.get_end_address()) {
					if (entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE) {
						auto f           = this->next_frame;
						this->next_frame = allocation_end_frame;
						return f.frame();
					} else {
						LOG(Log::TRACE, "bumping to %p", entry.get_end_address());
						this->next_frame = aligned<paddr_t>::aligned_up(entry.get_end_address());
						continue;
					}
				}
			}

			THROW(std::bad_alloc());
		}
	}
}   // namespace memory::physical_allocators
