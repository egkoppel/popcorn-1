/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc/serialization.hpp"

namespace ipc {
	SendChannel& operator<<(SendChannel& stream, const bool val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const uint8_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const uint16_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const uint32_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const uint64_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const int8_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const int16_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const int32_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const int64_t val) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const packet_start_t) {
		return stream;
	}
	SendChannel& operator<<(SendChannel& stream, const packet_end_t) {
		return stream;
	}
}   // namespace ipc
