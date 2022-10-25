/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_FRAME_MAIN_ALLOC_H
#define _HUGOS_FRAME_MAIN_ALLOC_H

#include <stdint.h>
#include "../main/multiboot.hpp"
#include "memory.h"
#include "allocator.hpp"

class FrameMainAllocator : public Allocator {
private:
	uint64_t *bitmap_start;
	uint64_t *bitmap_end;

	int set_bit(uint64_t addr);
	int get_bit(uint64_t addr);

public:
	FrameMainAlloc(uint64_t *bitmap_start, uint64_t *bitmap_end) : bitmap_start(bitmap_start), bitmap_end(bitmap_end) {}
	Option<uint64_t> allocate() override;
	Option<uint64_t> allocate_at(uint64_t) override;
	void deallocate(uint64_t addr) override;
};

#endif
