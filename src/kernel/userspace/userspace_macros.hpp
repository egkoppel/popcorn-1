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

unsigned long long __attribute__((naked))
_syscall5(unsigned long long syscallNo, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3, unsigned long long arg4, unsigned long long arg5);

#define sys_spawn_0(name, entry_func) (int)_syscall5(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)())entry_func), 0, 0, 0)
#define sys_spawn_1(name, entry_func, a1) (int)_syscall5(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)(uint64_t))entry_func), a1, 0, 0)
#define sys_spawn_2(name, entry_func, a1, a2) (int)_syscall5(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)(uint64_t,uint64_t))entry_func), a1, a2, 0)
#define sys_spawn_3(name, entry_func, a1, a2, a3) (int)_syscall5(syscall_vectors::spawn, (uint64_t)(name), (uint64_t)((void (*)(uint64_t,uint64_t,uint64_t))entry_func), a1, a2, a3)
#define journal_log(str, ...) { \
    /*char buf[128];*/            \
    /*snprintf(buf, 128, str, #__VA_ARGS__);*/ \
    (int)_syscall5(syscall_vectors::journal_log, (uint64_t)str, 0,0,0,0); \
}
#define sem_init(count) (void*)_syscall5(syscall_vectors::sem_new,count,0,0,0,0)
#define sem_destroy(sem) (int)_syscall5(syscall_vectors::sem_destroy,(uint64_t)sem,0,0,0,0)
#define sem_post(sem) (int)_syscall5(syscall_vectors::sem_post,(uint64_t)sem,0,0,0,0)
#define sem_wait(sem) (int)_syscall5(syscall_vectors::sem_wait,(uint64_t)sem,0,0,0,0)
#define sem_get_count(sem) (uint64_t)_syscall5(syscall_vectors::sem_get_count,(uint64_t)sem,0,0,0,0)
#define get_msg(buf) (uint64_t)_syscall5(syscall_vectors::get_msg,(uint64_t)buf,0,0,0,0)
#define wait_msg(buf) (uint64_t)_syscall5(syscall_vectors::wait_msg,(uint64_t)buf,0,0,0,0)
#define send_msg(pid, buf) (uint64_t)_syscall5(syscall_vectors::send_msg,(uint64_t)pid,(uint64_t)buf,0,0,0)
#define get_pid_by_name(name) (uint64_t)_syscall5(syscall_vectors::get_pid_by_name,(uint64_t)name,0,0,0,0)
#define yield() (uint64_t)_syscall5(syscall_vectors::yield,0,0,0,0,0)

#endif //HUG_USERSPACE_MACROS_HPP
