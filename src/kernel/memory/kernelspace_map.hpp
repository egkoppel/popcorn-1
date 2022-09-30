/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_KERNELSPACE_MAP_H
#define _HUGOS_KERNELSPACE_MAP_H

#include <utils.h>
#include "paging.h"

class KernelspaceMapper {
private:
	uint64_t next_addr;
	uint64_t max_addr = 0;
public:
	void set_max_addr(uint64_t max_addr) {
		KernelspaceMapper::max_addr = max_addr;
	}

public:
	KernelspaceMapper(uint64_t start_addr) : next_addr(ALIGN_UP(start_addr, 0x1000)) {};

	uint64_t map_to(uint64_t phys_addr, uint64_t size, entry_flags_t flags, allocator_vtable *allocator);
	uint64_t map(uint64_t size, entry_flags_t flags, allocator_vtable *allocator);
	uint64_t get_next_addr() const { return next_addr; }
};

#endif
