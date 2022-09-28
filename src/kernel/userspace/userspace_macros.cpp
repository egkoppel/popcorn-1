/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "userspace_macros.hpp"

unsigned long long __attribute__((naked))
_syscall5(unsigned long long syscallNo, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3, unsigned long long arg4, unsigned long long arg5) {
	__asm__ volatile("mov %r9, %rax; mov %r8, %r9; mov %rcx, %r8; syscall; ret;");
}
