
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

namespace ipc {
	class ServerImpl {
	protected:
		uint64_t extract_header();
		template<class... Args> bool extract_rpc_args(Args& ...args);

	private:
		RecvChannel channel;
	};

	template<class... Args> bool ServerImpl::extract_rpc_args(Args&...args) {
		(this->channel >> ... >> args)
				>> packet_end;
	}
}   // namespace ipc

#endif   // POPCORN_IDL_LIBIPC_SRC_SERVER_HPP
