
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_VM_MAP_HPP
#define HUGOS_VM_MAP_HPP

#include "physical_allocator.hpp"
#include "types.hpp"

#include <main/main.hpp>
#include <stdint.h>
#include <threading/task.hpp>
#include <utility/handle_table.hpp>
#include <utility>
#include <vector>

class VmMapping {
	/*public:
	enum vm_flags : uint64_t {
		PROT_READ  = 1ull << 1,
		PROT_WRITE = 1ull << 2,
		PROT_EXEC  = 1ull << 3,

		HINT_FAIL = 1ull << 4,
		COW       = 1ull << 5,
		TRANSFER  = 1ull << 6,
		NO_CACHE  = 1ull << 7
	};

private:
	uint64_t refcount = 0;
	syscall_handle_t handle;
	memory::FrameVector backing_frames;
	vm_flags flags_owner, flags_shared;
	threads::Task *_owner;

public:
	inline void increment_refcount() { this->refcount++; }
	void decrement_refcount(memory::IPhysicalAllocator& allocator);

	explicit VmMapping(memory::FrameVector backing_frames,
	                   vm_flags flags_owner,
	                   vm_flags flags_shared,
	                   threads::Task& owner) :
		handle(0),
		backing_frames(std::move(backing_frames)),
		flags_owner(flags_owner),
		flags_shared(flags_shared),
		_owner(&owner) {}

	inline void set_handle(syscall_handle_t handle) { this->handle = handle; }
	inline vm_flags owner_flags() const { return this->flags_owner; }
	inline vm_flags shared_flags() const { return this->flags_shared; }
	inline const threads::Task *owner() const { return this->_owner; }
	inline void set_owner(threads::Task& task) { this->_owner = &task; }*/
};

/*syscall_handle_t new_vm_mapping_anon(uint64_t size,
                                     VmMapping::vm_flags flags_owner,
                                     VmMapping::vm_flags flags_shared,
                                     threads::Task& owner,
                                     memory::IPhysicalAllocator& allocator);
syscall_handle_t new_vm_mapping(memory::PhysicalAddress phys_addr,
                                uint64_t size,
                                VmMapping::vm_flags flags_owner,
                                VmMapping::vm_flags flags_shared,
                                threads::Task& owner,
                                memory::IPhysicalContiguousAllocator& allocator);
VmMapping *get_vm_region(syscall_handle_t);*/

void vm_map_init();

#endif   //HUGOS_VM_MAP_HPP
