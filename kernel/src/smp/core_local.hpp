/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_CORE_LOCAL_HPP
#define HUGOS_CORE_LOCAL_HPP

#include "threading/scheduler.hpp"

#include <stdatomic.h>
#include <vector>

/*extern "C" struct core_local {
public:
	core_local *self = this;
	uint64_t idx     = atomic_fetch_add(&next_idx, 1);
	threads::ILocalScheduler& scheduler;

	static atomic_uint_fast64_t next_idx;

	core_local(threads::ILocalScheduler& scheduler);
};

extern std::vector<core_local *> core_local_data;

inline core_local *get_local_data() {
	core_local *ptr;
	__asm__ volatile("mov %%gs:0, %0" : "=r"(ptr));
	return ptr;
}*/

#endif   //HUGOS_CORE_LOCAL_HPP
