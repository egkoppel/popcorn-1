
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_IDL_LIBIPC_SRC_SERVER_HPP
#define POPCORN_IDL_LIBIPC_SRC_SERVER_HPP

#include "ipc.hpp"
#include "stddef.h"

namespace ipc {
	class ServerImpl {
	public:
		ServerImpl() = delete;

		[[noreturn]] void main_loop();

	protected:
		explicit ServerImpl(const char *address) : receiver(address, nullptr) {}
		virtual void handle_function(size_t function_hash, RecvChannel&) = 0;

		template<class... Args> static bool extract_rpc_args(RecvChannel& channel, Args&...args) {
			(channel >> ... >> args) >> packet_end;
		}

	private:
		convolution::ipc::Receiver receiver;
	};
}   // namespace ipc

#endif   // POPCORN_IDL_LIBIPC_SRC_SERVER_HPP
