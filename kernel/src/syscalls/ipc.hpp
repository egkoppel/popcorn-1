
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_IPC_HPP
#define HUGOS_IPC_HPP

#include <popcorn_prelude.h>

namespace syscalls::ipc {
	enum ipc_wait_flags { RET_ON_OPEN = 1 << 0, RET_ON_CLOSE = 1 << 1 };

	i64 register_address(const char *address, void *);
	void *open(const char *address);
	void close(void *channel);
	void *wait(i64 handle, ipc_wait_flags flags, usize ret);
	void notify(void *channel);
}   // namespace syscalls::ipc

#endif   // HUGOS_IPC_HPP
