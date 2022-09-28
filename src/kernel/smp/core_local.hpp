/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_CORE_LOCAL_HPP
#define HUG_CORE_LOCAL_HPP

#include <stdatomic.h>
#include "../threading/threading.hpp"

extern "C" struct core_local {
private:
	core_local *self = this;
	static atomic_uint_fast64_t next_idx;
public:
	uint64_t idx = atomic_fetch_add(&next_idx, 1);
	threads::Scheduler scheduler;

	core_local();
};

extern std::vector<core_local *> core_local_data;

inline core_local *get_local_data() {
	core_local *ptr;
	__asm__ volatile("mov %%gs:0, %0" : "=r"(ptr));
	return ptr;
}

#endif //HUG_CORE_LOCAL_HPP
