
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "virtual_region.hpp"

#include "virtual_allocator.hpp"

#include <main/main.hpp>

namespace memory {
	aligned<vaddr_t> general_allocator_t::allocate(u64 size) { return allocators.general_virtual().allocate(size); }
	void general_allocator_t::deallocate(aligned<vaddr_t> start, u64 size) noexcept {
		allocators.general_virtual().deallocate(start, size);
	}
}   // namespace memory
