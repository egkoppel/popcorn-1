/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MAIN_HPP
#define HUGOS_MAIN_HPP

#include <memory>

namespace memory {
	class IPhysicalAllocator;
}

struct kernel_allocators_t {
	std::unique_ptr<memory::IPhysicalAllocator> general_frame_allocator_ = nullptr;
	std::unique_ptr<memory::IPhysicalAllocator> dma_frame_allocator_     = nullptr;
	//std::unique_ptr<memory::IVirtualAllocator> kernel

	constexpr memory::IPhysicalAllocator& general() const noexcept { return *this->general_frame_allocator_; }
	constexpr memory::IPhysicalAllocator& dma() const noexcept { return *this->dma_frame_allocator_; }
};
extern kernel_allocators_t allocators;

#endif
