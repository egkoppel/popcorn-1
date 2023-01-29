
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "irq.hpp"

#include <threading/scheduler.hpp>

bad_dict_irq irq_list;

namespace syscall {
	/* TODO: Make this actually use irq line instead of preset cpu vector */
	i64 register_isa_irq(i64 irq) {
		// auto gsi    = /* TODO */;
		// auto vector = /* TODO */;
		if (irq != 1) return -10;
		irq_list.emplace_back(0x96, *threads::local_scheduler->get_current_task());
		return 0;
	}

	i64 unregister_isa_irq(i64) {
		return -1;
	}
}   // namespace syscall
