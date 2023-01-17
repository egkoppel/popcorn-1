
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
		Receiver(const char* address, void*) : handle(convolution_ipc_register(address, nullptr)) {
			if (this->handle <= 0) throw std::runtime_error("Failed to register IPC receiver");
		}

		void wait() {
			convolution_ipc_wait(this->handle, );
		}

	private:
		convolution_handle handle;
	};
}   // namespace convolution

#endif   // POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_IPC_HPP
