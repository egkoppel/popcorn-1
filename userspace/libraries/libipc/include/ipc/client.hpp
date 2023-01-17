
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_IDL_LIBIPC_SRC_CLIENT_HPP
#define POPCORN_IDL_LIBIPC_SRC_CLIENT_HPP

#include "ipc.hpp"

namespace ipc {
	class ClientImpl {
	protected:
		template<class... Args> void send_rpc(uint64_t function_hash, Args&&...args);

	private:
		SendChannel channel;
	};

	template<class... Args> void ClientImpl::send_rpc(uint64_t function_hash, Args&&...args) {
		((this->channel << packet_start << function_hash) << ... << args) << packet_end;
	}
}   // namespace ipc

#endif   // POPCORN_IDL_LIBIPC_SRC_CLIENT_HPP
