
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_VM_MAP_HPP
#define HUG_VM_MAP_HPP

#include <stdint.h>
#include "../interrupts/syscall.hpp"
#include <vector>
#include "allocator.h"
#include "../main/main.hpp"
#include <utility>

class VmMapping {
private:
	uint64_t refcount = 0;
	syscall_handle_t handle;
	std::vector<uint64_t> backing_frames;
public:
	inline void increment_refcount() { this->refcount++; }
	inline void decrement_refcount() {
		this->refcount--;
		if (this->refcount == 0) {
			for (auto frame : backing_frames) {
				allocator_deallocate(global_frame_allocator, frame);
			}
		}
	}

	explicit VmMapping(std::vector<uint64_t> backing_frames) : backing_frames(std::move(backing_frames)), handle(0) {}

	inline void set_handle(syscall_handle_t handle) { this->handle = handle; }
};

syscall_handle_t new_vm_mapping_anon(uint64_t size);
syscall_handle_t new_vm_mapping(uint64_t phys_addr, uint64_t size);

void vm_map_init();

#endif //HUG_VM_MAP_HPP
