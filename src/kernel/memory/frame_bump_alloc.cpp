/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "frame_bump_alloc.hpp"
#include <utils.h>
#include <stdio.h>

Option<uint64_t> FrameBumpAllocator::allocate() {
	uint64_t attempt = ALIGN_UP(this->next_alloc, 0x1000);

	while (true) {
		fprintf(stdserial, "Attempt alloc at %p\n", attempt);
		if (attempt >= this->kernel_start && attempt < this->kernel_end) {
			fprintf(stdserial, "bump alloc kernel jump\n");
			attempt = ALIGN_UP(this->kernel_end, 0x1000);
			continue;
		}

		if (attempt >= this->multiboot_start && attempt < this->multiboot_end) {
			fprintf(stdserial, "bump alloc multiboot jump\n");
			attempt = ALIGN_UP(this->multiboot_end, 0x1000);
			continue;
		}

		for (multiboot::memory_map_entry entry : *this->mem_map) {
			if (entry.base_addr <= attempt && attempt < entry.base_addr + entry.length) {
				if (entry.type == multiboot::memory_type::AVAILABLE) {
					this->next_alloc = attempt + 0x1000;
					return Some<uint64_t>(attempt);
				} else {
					attempt = ALIGN_UP(entry.base_addr + entry.length, 0x1000);
					break;
				}
			}
		}
		attempt = ALIGN_UP(attempt + 0x1000, 0x1000);
	}
}
