
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_IDL_LIBIPC_INCLUDE_IPC_HPP
#define POPCORN_IDL_LIBIPC_INCLUDE_IPC_HPP

#include "serialization.hpp"

#include <convolution/ipc.hpp>

namespace ipc {
	class SendChannel {
		friend SendChannel& operator<<(SendChannel&, bool);
		friend SendChannel& operator<<(SendChannel&, uint8_t);
		friend SendChannel& operator<<(SendChannel&, uint16_t);
		friend SendChannel& operator<<(SendChannel&, uint32_t);
		friend SendChannel& operator<<(SendChannel&, uint64_t);
		friend SendChannel& operator<<(SendChannel&, int8_t);
		friend SendChannel& operator<<(SendChannel&, int16_t);
		friend SendChannel& operator<<(SendChannel&, int32_t);
		friend SendChannel& operator<<(SendChannel&, int64_t);
		friend SendChannel& operator<<(SendChannel&, packet_start_t);
		friend SendChannel& operator<<(SendChannel&, packet_end_t);

	public:
		explicit SendChannel(const char *address) : sender(address) {}

	private:
		convolution::ipc::Sender sender;
	};

	class RecvChannel {
		friend RecvChannel& operator>>(RecvChannel&, bool&);
		friend RecvChannel& operator>>(RecvChannel&, uint8_t&);
		friend RecvChannel& operator>>(RecvChannel&, uint16_t&);
		friend RecvChannel& operator>>(RecvChannel&, uint32_t&);
		friend RecvChannel& operator>>(RecvChannel&, uint64_t&);
		friend RecvChannel& operator>>(RecvChannel&, int8_t&);
		friend RecvChannel& operator>>(RecvChannel&, int16_t&);
		friend RecvChannel& operator>>(RecvChannel&, int32_t&);
		friend RecvChannel& operator>>(RecvChannel&, int64_t&);
		friend RecvChannel& operator>>(RecvChannel&, packet_start_t);
		friend RecvChannel& operator>>(RecvChannel&, packet_end_t);

	public:
		explicit RecvChannel(void *start) : start(start) {}

	protected:
		void *start;
	};
}   // namespace ipc

#endif   // POPCORN_IDL_LIBIPC_INCLUDE_IPC_HPP
