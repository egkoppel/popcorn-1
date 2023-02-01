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
			if ((this->next_frame >= this->kernel_start_frame && this->next_frame < this->kernel_end_frame)
			    || (this->next_frame >= allocation_end_frame && allocation_end_frame < this->kernel_end_frame)) {
				// fprintf(stdserial, "bump alloc kernel jump\n");
				this->next_frame = this->kernel_end_frame;
				continue;
			}

			if ((this->next_frame >= this->multiboot_start_frame && this->next_frame < this->multiboot_end_frame)
			    || (this->next_frame >= allocation_end_frame && allocation_end_frame < this->multiboot_end_frame)) {
				// fprintf(stdserial, "bump alloc multiboot jump\n");
				this->next_frame = this->multiboot_end_frame;
				continue;
			}

			if ((this->next_frame >= this->ramdisk_start_frame && this->next_frame < this->ramdisk_end_frame) 
			    || (this->next_frame >= allocation_end_frame && allocation_end_frame < this->ramdisk_end_frame)) {
				this->next_frame = this->ramdisk_end_frame; 
				continue; 
			}   

			for (auto entry : *this->mem_map) {
				if (entry.get_start_address() <= this->next_frame && allocation_end_frame < entry.get_end_address()) {
					if (entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE) {
						auto f           = this->next_frame;
						this->next_frame = allocation_end_frame;
						return f.frame();
					} else {
						this->next_frame = aligned<paddr_t>::aligned_up(entry.get_end_address());
						continue;
					}
				}
			}

			THROW(std::bad_alloc());
		}
	}
}   // namespace memory::physical_allocators
