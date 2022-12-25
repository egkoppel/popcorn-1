
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_MEMORY_VIRTUAL_ALLOCATORS_TREE_ALLOC_HPP
#define HUGOS_KERNEL_SRC_MEMORY_VIRTUAL_ALLOCATORS_TREE_ALLOC_HPP

#include "../types.hpp"
#include "../virtual_allocator.hpp"

#include <cstdint>
#include <utility/range_tree.hpp>

namespace memory::virtual_allocators {
	class TreeAllocator : public IVirtualAllocator {
	private:
		structures::avl_range_map<int, int> tree;

		void foo() { this->tree.insert(5, 6, 7); }

	public:
	};
}   // namespace memory::virtual_allocators

#endif   //HUGOS_KERNEL_SRC_MEMORY_VIRTUAL_ALLOCATORS_TREE_ALLOC_HPP
