
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_SYSCALL_H
#define POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t convolution_handle;

enum convolution_syscall_vectors {
	convolution_get_current_task_vector = 0x000, /* syscall_handle_t get_current_task() */
	convolution_yield_vector            = 0x001, /* int64_t yield() */
	convolution_exit_vector             = 0x002, /* void exit() */
	convolution_sleep_vector            = 0x003, /* int64_t sleep(uint64_t) */
	convolution_suspend_vector          = 0x004, /* int64_t suspend() */
	convolution_resume_vector           = 0x005, /* int64_t resume(syscall_handle_t) */
	convolution_spawn_vector = 0x006, /* syscall_handle_t spawn(char*, void(*)(uint64_t, uint64_t, uint64_t), uint64_t,
	                                   * uint64_t, uint64_t)
	                                   */
	convolution_spawn2_vector        = 0x007, /* syscall_handle_t spawn(char*, char**, char**) */
	convolution_get_time_used_vector = 0x008, /* int64_t get_time_used() */

	convolution_mailbox_new_vector             = 0x200,
	convolution_mailbox_send_vector            = 0x201,
	convolution_mailbox_recv_vector            = 0x202,
	convolution_mailbox_destroy_vector         = 0x203,
	convolution_mailbox_transfer_vector        = 0x204,
	convolution_mailbox_reply_vector           = 0x205,
	convolution_mailbox_send_with_reply_vector = 0x206,

	convolution_region_new_vector      = 0x300,
	convolution_region_new_anon_vector = 0x301,
	convolution_region_new_dma_vector  = 0x302,
	convolution_set_flags_vector       = 0x303,
	convolution_map_region_vector      = 0x304,
	convolution_share_region_vector    = 0x306,

	convolution_mutex_lock_vector,
	convolution_mutex_try_lock_vector,
	convolution_mutex_unlock_vector,
	convolution_mutex_new_vector,
	convolution_mutex_destroy_vector,

	convolution_sem_post_vector,
	convolution_sem_wait_vector,
	convolution_sem_get_count_vector,
	convolution_sem_new_vector,
	convolution_sem_destroy_vector,

	convolution_ipc_register_vector,
	convolution_ipc_open_vector,
	convolution_ipc_close_vector,
	convolution_ipc_wait_vector,
	convolution_ipc_notify_vector
};

__attribute__((naked)) static inline void convolution_syscall_void_void(uint64_t vector) {}
__attribute__((naked)) static inline convolution_handle convolution_syscall_handle_void(uint64_t vector) {}
__attribute__((naked)) static inline convolution_handle convolution_syscall_handle_ptr_ptr(uint64_t vector,
                                                                                           void *,
                                                                                           void *) {}
__attribute__((naked)) static inline void *convolution_syscall_ptr_ptr(uint64_t vector, void *) {}
__attribute__((naked)) static inline void convolution_syscall_void_ptr(uint64_t vector, void *) {}
__attribute__((naked)) static inline void *convolution_syscall_ptr_void(uint64_t vector) {}
__attribute__((naked)) static inline void *convolution_syscall_ptr_handle_flags_ptr(uint64_t vector,
                                                                                    convolution_handle,
                                                                                    uint64_t,
                                                                                    void *) {}
__attribute__((naked)) static inline int64_t convolution_syscall_code_ptr(uint64_t vector, void *) {}

#ifdef __cplusplus
};
#endif

#endif   // POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_SYSCALL_H
