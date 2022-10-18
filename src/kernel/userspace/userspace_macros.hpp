/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_USERSPACE_MACROS_HPP
#define HUG_USERSPACE_MACROS_HPP

#include "../interrupts/syscall.hpp"
#include "../threading/mailing.hpp"

template<class R, class A1 = int64_t, class A2 = int64_t, class A3 = int64_t, class A4 = int64_t, class A5 = int64_t>
R __attribute__((naked))
_syscall5(unsigned long long syscallNo, A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 arg5) {
	__asm__ volatile("mov %r9, %rax; mov %r8, %r9; mov %rcx, %r8; syscall; ret;");
}

#define sys_spawn_0(name, entry_func) _syscall5<int64_t>(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)())entry_func), 0, 0, 0)
#define sys_spawn_1(name, entry_func, a1) _syscall5<int64_t>(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)(uint64_t))entry_func), a1, 0, 0)
#define sys_spawn_2(name, entry_func, a1, a2) _syscall5<int64_t>(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)(uint64_t,uint64_t))entry_func), a1, a2, 0)
#define sys_spawn_3(name, entry_func, a1, a2, a3) _syscall5<int64_t>(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)(uint64_t,uint64_t,uint64_t))entry_func), a1, a2, a3)

#define sem_init(count) _syscall5<void*>(syscall_vectors::sem_new,count,0,0,0,0)
#define sem_destroy(sem) _syscall5<int>(syscall_vectors::sem_destroy,(uint64_t)(sem),0,0,0,0)
#define sem_post(sem) _syscall5<int>(syscall_vectors::sem_post,(uint64_t)(sem),0,0,0,0)
#define sem_wait(sem) _syscall5<int>(syscall_vectors::sem_wait,(uint64_t)(sem),0,0,0,0)
#define sem_get_count(sem) _syscall5<uint64_t>(syscall_vectors::sem_get_count,(uint64_t)(sem),0,0,0,0)

#define mbox_new() _syscall5<int64_t>(syscall_vectors::mailbox_new, 0,0,0,0,0)
#define mbox_destroy(handle) _syscall5<int64_t>(syscall_vectors::mailbox_destroy, handle,0,0,0,0)
#define recv_msg(handle, timeout, buf) _syscall5<int64_t>(syscall_vectors::mailbox_recv,handle,timeout,buf,0,0)
#define send_msg(handle, timeout, buf) _syscall5<int64_t>(syscall_vectors::mailbox_send,handle,timeout,buf,0,0)

#define yield() _syscall5<int64_t>(syscall_vectors::yield,0,0,0,0,0)

#endif //HUG_USERSPACE_MACROS_HPP
