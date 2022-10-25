
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
#include "../threading/threading.hpp"

class VmMapping {
public:
	enum vm_flags : uint64_t {
		PROT_READ = 1ull << 1,
		PROT_WRITE = 1ull << 2,
		PROT_EXEC = 1ull << 3,

		HINT_FAIL = 1ull << 4,
		COW = 1ull << 5,
		TRANSFER = 1ull << 6,
		NO_CACHE = 1ull << 7
	};
private:
	uint64_t refcount = 0;
	syscall_handle_t handle;
	std::vector<uint64_t> backing_frames;
	vm_flags flags_owner, flags_shared;
	std::shared_ptr<threads::Task> owner;
public:
	inline void increment_refcount() { this->refcount++; }
	void decrement_refcount();

	explicit VmMapping(std::vector<uint64_t> backing_frames, vm_flags flags_owner, vm_flags flags_shared, std::shared_ptr<threads::Task> owner) : backing_frames(std::move(backing_frames)), handle(0),
	                                                                                                                                              flags_owner(flags_owner),
	                                                                                                                                              flags_shared(flags_shared), owner(std::move(owner)) {}

	inline void set_handle(syscall_handle_t handle) { this->handle = handle; }
	inline vm_flags get_flags_owner() const { return this->flags_owner; }
	inline vm_flags get_flags_shared() const { return this->flags_shared; }
	inline const std::shared_ptr<threads::Task>& get_owner() const { return this->owner; }
	inline void set_owner(std::shared_ptr<threads::Task> task) { this->owner = std::move(task); }
	inline const std::vector<uint64_t>& get_frames() const { return this->backing_frames; }
};

syscall_handle_t new_vm_mapping_anon(uint64_t size, VmMapping::vm_flags flags_owner, VmMapping::vm_flags flags_shared, std::shared_ptr<threads::Task> owner);
syscall_handle_t new_vm_mapping(uint64_t phys_addr, uint64_t size, VmMapping::vm_flags flags_owner, VmMapping::vm_flags flags_shared, std::shared_ptr<threads::Task> owner);
VmMapping *get_vm_region(syscall_handle_t);

void vm_map_init();

#endif //HUG_VM_MAP_HPP
