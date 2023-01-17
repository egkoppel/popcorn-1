
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_IDL_LIBIPC_SRC_SERIALIZATION_HPP
#define POPCORN_IDL_LIBIPC_SRC_SERIALIZATION_HPP

#include "packet.hpp"

#include <stdint.h>

namespace ipc {
	class SendChannel;
	class RecvChannel;

	SendChannel& operator<<(SendChannel&, bool);
	SendChannel& operator<<(SendChannel&, uint8_t);
	SendChannel& operator<<(SendChannel&, uint16_t);
	SendChannel& operator<<(SendChannel&, uint32_t);
	SendChannel& operator<<(SendChannel&, uint64_t);
	SendChannel& operator<<(SendChannel&, int8_t);
	SendChannel& operator<<(SendChannel&, int16_t);
	SendChannel& operator<<(SendChannel&, int32_t);
	SendChannel& operator<<(SendChannel&, int64_t);
	SendChannel& operator<<(SendChannel&, packet_start_t);
	SendChannel& operator<<(SendChannel&, packet_end_t);

	RecvChannel& operator>>(RecvChannel&, bool&);
	RecvChannel& operator>>(RecvChannel&, uint8_t&);
	RecvChannel& operator>>(RecvChannel&, uint16_t&);
	RecvChannel& operator>>(RecvChannel&, uint32_t&);
	RecvChannel& operator>>(RecvChannel&, uint64_t&);
	RecvChannel& operator>>(RecvChannel&, int8_t&);
	RecvChannel& operator>>(RecvChannel&, int16_t&);
	RecvChannel& operator>>(RecvChannel&, int32_t&);
	RecvChannel& operator>>(RecvChannel&, int64_t&);
	RecvChannel& operator>>(RecvChannel&, packet_start_t);
	RecvChannel& operator>>(RecvChannel&, packet_end_t);
}   // namespace ipc

#endif   // POPCORN_IDL_LIBIPC_SRC_SERIALIZATION_HPP
