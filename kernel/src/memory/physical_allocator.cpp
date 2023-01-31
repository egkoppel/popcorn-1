/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "physical_allocator.hpp"

#include <cstdio>
#include <log.hpp>

namespace memory {
	frame_t *IPhysicalAllocator::allocate(u64 size) {
		LOG(Log::DEBUG, "Physical allocation requested of size %lx", size);
		auto ret = this->allocate_(size);
		LOG(Log::DEBUG, "Allocated at %lp", ret->addr());
		ret->allocated_by = this;
		ret->ref_count++;
		return ret;
	}

	frame_t *IPhysicalAllocator::allocate(aligned<paddr_t> at, u64 size) {
		LOG(Log::DEBUG, "Physical allocation requested of size %lx at %lp", size, at);
		auto ret = this->allocate_at_(at, size);
		LOG(Log::DEBUG, "Allocated at %zx", (ret - mem_map) * constants::frame_size);
		ret->allocated_by = this;
		ret->ref_count++;
		return ret;
	}

	void IPhysicalAllocator::drop(frame_t *start, u64 size) noexcept {
		LOG(Log::DEBUG, "drop ref at paddr %lp\nrc: %llu", start->addr(), start->ref_count);
		if (--start->ref_count == 0) start->allocated_by->deallocate_(start, size);
	}
}   // namespace memory
