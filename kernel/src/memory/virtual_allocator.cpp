/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "virtual_allocator.hpp"

namespace memory {
	aligned<vaddr_t> IVirtualAllocator::allocate(uint64_t byte_length) { return this->allocate_(byte_length); }
	void IVirtualAllocator::deallocate(aligned<vaddr_t> start, uint64_t byte_length) noexcept {
		this->deallocate_(start, byte_length);
	}
}   // namespace memory
