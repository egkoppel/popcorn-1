
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vm_map.hpp"
#include "paging.h"
#include "../main/main.hpp"
#include <utility>
#include <utils.h>

alignas(alignof(SyscallHandleTable<VmMapping, syscall_handle_type::syscall_handle_type::VM>)) static char vm_map_handles_list_[sizeof(SyscallHandleTable<VmMapping, syscall_handle_type::syscall_handle_type::VM>)]; // memory for the stream object
auto& vm_map_handles_list = reinterpret_cast<SyscallHandleTable<VmMapping, syscall_handle_type::syscall_handle_type::VM>&>(vm_map_handles_list_);

void vm_map_init() {
	new(&vm_map_handles_list) SyscallHandleTable<VmMapping, syscall_handle_type::syscall_handle_type::VM>();
}

syscall_handle_t new_vm_mapping_anon(uint64_t size, VmMapping::vm_flags flags_owner, VmMapping::vm_flags flags_shared, std::shared_ptr<threads::Task> owner, Allocator *allocator) {
	auto backing_frames = std::vector<uint64_t>(IDIV_ROUND_UP(size, 0x1000));
	for (uint64_t i = 0; i < size; i += 0x1000) {
		auto frame = allocator->allocate();
		if (frame.is_none()) {
			for (auto frame : backing_frames) {
				allocator->deallocate(frame);
			}
			backing_frames.clear();
			return -1;
		}
		backing_frames.push_back(global_frame_allocator->allocate().unwrap());
	}

	auto handle = vm_map_handles_list.new_handle(VmMapping(std::move(backing_frames), flags_owner, flags_shared, owner));
	auto data = vm_map_handles_list.get_data_from_handle_ptr(handle);
	data->set_handle(handle);
	return handle;
}

syscall_handle_t new_vm_mapping(uint64_t phys_addr, uint64_t size, VmMapping::vm_flags flags_owner, VmMapping::vm_flags flags_shared, std::shared_ptr<threads::Task> owner, Allocator *allocator) {
	if (phys_addr & (~0x1000)) return 0;

	auto backing_frames = std::vector<uint64_t>(IDIV_ROUND_UP(size, 0x1000));
	for (uint64_t i = phys_addr; i < size; i += 0x1000) {
		auto frame = allocator->allocate_at(i);
		if (frame.is_none()) {
			for (auto frame : backing_frames) {
				allocator->deallocate(frame);
			}
			backing_frames.clear();
			return -1;
		}
		backing_frames.push_back(frame.unwrap());
	}

	auto handle = vm_map_handles_list.new_handle(VmMapping(std::move(backing_frames), flags_owner, flags_shared, owner));
	auto data = vm_map_handles_list.get_data_from_handle_ptr(handle);
	data->set_handle(handle);
	return handle;
}

VmMapping *get_vm_region(syscall_handle_t handle) {
	return vm_map_handles_list.get_data_from_handle_ptr(handle);
}

inline void VmMapping::decrement_refcount(Allocator *allocator) {
	this->refcount--;
	if (this->refcount == 0) {
		for (auto frame : this->backing_frames) {
			allocator->deallocate(frame);
		}
		vm_map_handles_list.free_handle(this->handle);
	}
}

int increment_vm_mapping_refcount(syscall_handle_t handle) {
	if (auto vm_map = vm_map_handles_list.get_data_from_handle_ptr(handle)) {
		vm_map->increment_refcount();
		return 0;
	} else return -1;
}

int decrement_vm_mapping_refcount(syscall_handle_t handle) {
	if (auto vm_map = vm_map_handles_list.get_data_from_handle_ptr(handle)) {
		vm_map->decrement_refcount(nullptr);
		return 0;
	} else return -1;
}
