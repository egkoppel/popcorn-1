
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_VM_HPP
#define HUGOS_VM_HPP

#include "memory/types.hpp"

#include <cstdint>
#include <memory/vm_map.hpp>
#include <smp/core_local.hpp>

namespace syscalls::vm {
	/*extern inline int64_t new_region(memory::PhysicalAddress physical_address,
	                                 uint64_t size,
	                                 VmMapping::vm_flags owner_flags,
	                                 VmMapping::vm_flags share_flags) {
		return new_vm_mapping(physical_address,
		                      size,
		                      owner_flags,
		                      share_flags,
		                      *get_local_data()->scheduler.get_current_task(),
		                      allocators.general_contiguous());
	}*/
}   // namespace syscalls::vm

#endif   //HUGOS_VM_HPP
