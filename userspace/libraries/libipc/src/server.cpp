
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc/server.hpp"

#include <convolution/ipc.hpp>

namespace ipc {
	namespace {
		uint64_t extract_header(RecvChannel& channel) {
			uint64_t function_hash;
			channel >> packet_start >> function_hash;
			return function_hash;
		}
	}   // namespace

	void ServerImpl::main_loop() {
		while (true) {
			this->receiver.wait();
			void *buffer_ptr = 0 /* TODO */;
			RecvChannel buffer{buffer_ptr};
			uint64_t function_hash = extract_header(buffer);
			this->handle_function(function_hash, buffer);
		}
	}
}   // namespace ipc
