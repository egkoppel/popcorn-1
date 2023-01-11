
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_SYSCALLS_IRQ_HPP
#define POPCORN_KERNEL_SRC_SYSCALLS_IRQ_HPP

#include <popcorn_prelude.h>

namespace syscall {
	i64 register_isa_irq(i64);
	i64 unregister_isa_irq(i64);
}

#endif   // POPCORN_KERNEL_SRC_SYSCALLS_IRQ_HPP
