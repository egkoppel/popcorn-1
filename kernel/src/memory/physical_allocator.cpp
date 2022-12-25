/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "physical_allocator.hpp"

namespace memory {
	frame_t *IPhysicalAllocator::allocate(u64 size) {
		auto ret          = this->allocate_(size);
		ret->allocated_by = this;
		return ret;
	}
	void IPhysicalAllocator::deallocate(const frame_t *start, u64 size) noexcept {
		start->allocated_by->deallocate_(start, size);
	}
}   // namespace memory
