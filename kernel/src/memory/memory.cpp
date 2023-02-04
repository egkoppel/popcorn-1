/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "paging.hpp"
#include "types.hpp"

#include <cassert>
#include <cstdio>
#include <log.hpp>
#include <main/main.hpp>

using namespace memory;

static bool initialised = false;

void memory::init_sbrk() {
	initialised = true;
}

extern "C" void *sbrk(intptr_t increment) noexcept try {
	static aligned<vaddr_t> current_break = vaddr_t{.address = constants::kernel_heap_start};
	if (!initialised) return (void *)-1;

	vaddr_t ret                = current_break;
	intptr_t rounded_increment = IDIV_ROUND_UP(increment, constants::frame_size);
	assert_msg((increment & (0x1000 - 1)) == 0, "welp - it's not aligned");
	rounded_increment          = increment / 4096;
	aligned<vaddr_t> new_break = current_break + rounded_increment;
	LOG(Log::DEBUG, "sbrk old: %p, new: %p", ret, new_break);

	if (current_break < new_break) {
		if (!(new_break.address.address < constants::kernel_heap_end)) return (void *)-1;
		auto backing_frames = allocators.general().allocate(rounded_increment * constants::frame_size);

		for (aligned<vaddr_t> page_to_map = current_break; page_to_map < new_break; page_to_map++, backing_frames++) {
			using enum paging::PageTableFlags;
			auto flags = WRITEABLE | NO_EXECUTE | GLOBAL;

			paging::kas.map_page_to(page_to_map, backing_frames, flags);
		}
	} else if (current_break > new_break) {
		if (!(new_break.address.address > constants::kernel_heap_start)) return (void *)-1;
		for (aligned<vaddr_t> page_to_unmap = new_break; page_to_unmap < current_break; page_to_unmap++) {
			paging::kas.unmap_page(page_to_unmap);
		}
	}
	current_break = new_break;

	LOG(Log::DEBUG, "Heap adjusted");

	return (void *)ret;
} catch (std::bad_alloc&) { return (void *)-1; }
