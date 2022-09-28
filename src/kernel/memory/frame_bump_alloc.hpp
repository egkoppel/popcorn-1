/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_FRAME_BUMP_ALLOC_H
#define _HUGOS_FRAME_BUMP_ALLOC_H

#include <stdint.h>
#include "../main/multiboot.hpp"
#include "memory.h"
#include "allocator.h"

struct frame_bump_alloc_state {
	allocator_vtable vtable;
	uint64_t next_alloc;
	uint64_t kernel_start;
	uint64_t kernel_end;
	uint64_t multiboot_start;
	uint64_t multiboot_end;
	multiboot::memory_map_tag *mem_map;

	static uint64_t bump_alloc_allocate(frame_bump_alloc_state *self);
	uint64_t allocate();
};

extern const allocator_vtable frame_bump_alloc_state_vtable;

#endif
