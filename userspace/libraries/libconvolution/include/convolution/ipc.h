
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_H
#define POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_H

#include "syscall.h"

static inline convolution_handle convolution_ipc_register(const char *address, void *caps) {
	return convolution_syscall_handle(convolution_ipc_register_vector);
}

static inline void *convolution_ipc_open(const char *address) {
	return convolution_syscall_ptr(convolution_ipc_open_vector, address);
}

static inline void convolution_ipc_close(void *buffer) {
	convolution_syscall(convolution_ipc_close_vector, buffer);
}

static inline void *convolution_ipc_wait(convolution_handle ipc_channel, uint64_t flags, uint64_t *ret) {
	return convolution_syscall_ptr(convolution_ipc_wait_vector, ipc_channel, flags, ret);
}

static inline int64_t convolution_ipc_notify(void *buffer) {
	return convolution_syscall(convolution_ipc_notify_vector, buffer);
}

#endif   // POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_H
