
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_DOUBLE_FAULT_HPP
#define HUGOS_DOUBLE_FAULT_HPP

#include <arch/interrupts.hpp>

namespace interrupt_handlers {
	[[noreturn]] void double_fault(arch::interrupt_info_t *) noexcept;
}

#endif   //HUGOS_DOUBLE_FAULT_HPP
