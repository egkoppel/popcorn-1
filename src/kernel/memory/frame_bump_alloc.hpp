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
#include "allocator.hpp"

class FrameBumpAllocator : public Allocator {
private:
	uint64_t next_alloc;
	uint64_t kernel_start;
	uint64_t kernel_end;
	uint64_t multiboot_start;
	uint64_t multiboot_end;
	multiboot::memory_map_tag *mem_map;
public:
	FrameBumpAllocator(uint64_t kernel_start, uint64_t kernel_end, uint64_t multiboot_start, uint64_t multiboot_end, multiboot::memory_map_tag *mem_map) : next_alloc(0),
	                                                                                                                                                       kernel_start(kernel_start),
	                                                                                                                                                       kernel_end(kernel_end),
	                                                                                                                                                       multiboot_start(multiboot_start),
	                                                                                                                                                       multiboot_end(multiboot_end),
	                                                                                                                                                       mem_map(mem_map) {}

	Option<uint64_t> allocate() override;
};

#endif
