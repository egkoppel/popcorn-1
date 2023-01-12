
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "userspace_ps2_keyboard.hpp"

#include "userspace_macros.hpp"

namespace driver::ps2_keyboard {
	int main() {
		auto err = _syscall_new(SyscallVectors::register_isa_irq, 0x1);
		if (err < 0) {
			LOG(Log::CRITICAL, "failed to register keyboard irq");
			while (true) __asm__ volatile("nop");
		}
		while (true) {
			_syscall_new(SyscallVectors::suspend);
			LOG(Log::WARNING, "keyboard irq");
		}
	}
}   // namespace driver::ps2_keyboard
