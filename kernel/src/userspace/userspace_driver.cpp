
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "userspace_driver.hpp"

void driver::_start(usize main_entry) {
	__asm__ volatile(
			"movq %rdi, %rbx;"     // save entrypoint
			"movq $0x100, %rax;"   // syscall number - 0x100 : make_stack
			"syscall;"
			"mov %rax, %rsp;"   // load stack pointer
			"call *%rbx;");
}
