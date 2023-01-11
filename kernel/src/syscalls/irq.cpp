
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

namespace syscall {
	i64 register_isa_irq(i64) {
		return 0;
	}

	i64 unregister_isa_irq(i64) {
		return 0;
	}
}   // namespace syscall
