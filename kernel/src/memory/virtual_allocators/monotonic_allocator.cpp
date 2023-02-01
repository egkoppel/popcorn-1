
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "monotonic_allocator.hpp"

#include <new>

namespace memory::virtual_allocators {
	aligned<vaddr_t> MonotonicAllocator::allocate_(u64 byte_length) {
		auto start = this->current_;
		auto end   = start + IDIV_ROUND_UP(byte_length, constants::frame_size);
		if (end > this->end_) THROW(std::bad_alloc())
		else {
			this->current_ = end;
			return start;
		}
	}

	aligned<vaddr_t> MonotonicAllocator::allocate_at_(aligned<vaddr_t> start, uint64_t byte_length) {
		LOG(Log::DEBUG, "Monotonic virtual allocator currently at %lp", this->current_);
		if (start >= this->current_) {
			auto end = start + IDIV_ROUND_UP(byte_length, constants::frame_size);
			if (end > this->end_) {
				THROW(std::bad_alloc());
			} else {
				this->current_ = end;
				return start;
			}
		} else {
			THROW(std::bad_alloc());
		}
	}
}   // namespace memory::virtual_allocators
