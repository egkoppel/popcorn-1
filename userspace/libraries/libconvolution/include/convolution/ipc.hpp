
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_HPP
#define POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_HPP

#include "ipc.h"

namespace convolution::ipc {
	class Receiver {
	public:
		Receiver(const char *address, void *) : handle(convolution_ipc_register(address, nullptr)) {
			// TODO: if (this->handle <= 0) throw std::runtime_error("Failed to register IPC receiver");
		}

		enum class wait_return_reason { new_message, opened_channel, closed_channel };

		wait_return_reason wait(bool return_on_channel_change = false) const {
			uint64_t ret;
			void *ptr = convolution_ipc_wait(this->handle, return_on_channel_change ? 3 : 0, &ret);
			if (ret & 1) return wait_return_reason::opened_channel;
			else if (ret & 2) return wait_return_reason::closed_channel;
			else return wait_return_reason::new_message;
		}

	private:
		convolution_handle handle;
	};
}   // namespace convolution::ipc

#endif   // POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_HPP
