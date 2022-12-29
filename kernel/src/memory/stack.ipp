/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "paging.hpp"
#include "physical_allocator.hpp"

#include <assert.h>
#include <cstdio>
#include <utility/skip.hpp>
#include <utility/zip.hpp>

namespace memory {
	template<class VAllocator>
	KStack<VAllocator>::KStack(usize stack_size, IPhysicalAllocator& pallocator, VAllocator&& vallocator) :
		backing_region{stack_size, pallocator},
		virtual_region{stack_size + constants::frame_size, std::forward<VAllocator>(vallocator)} {
		using enum paging::PageTableFlags;
		auto flags = WRITEABLE | NO_EXECUTE;

		LOG(Log::DEBUG,
		    "Creating new stack: %p -> %p - Guard at %p",
		    *this->virtual_region.begin() + 1,
		    *this->virtual_region.end(),
		    *this->virtual_region.begin());

		for (auto&& [frame, page] : iter::zip(this->backing_region, iter::skip<1>(this->virtual_region))) {
			paging::kas.map_page_to(page, frame, flags);
		}

		// FIXME: Fix guard page
		// paging::current_page_table->mark_for_no_map(*this->virtual_region.begin(), frame_allocator);   // Guard page
	}

	template<class VAllocator> KStack<VAllocator>::~KStack() {
		LOG(Log::DEBUG, "Stack (%p -> %p) dropped", this->bottom(), this->top());

		for (auto page : this->virtual_region) { paging::kas.unmap_page(page); }

		// FIXME: Fix guard page
		// paging::unmark_for_no_map(this->start_page - 1);   // Guard page
	}
}   // namespace memory
