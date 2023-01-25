
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_IDL_LIBIPC_SRC_PACKET_HPP
#define POPCORN_IDL_LIBIPC_SRC_PACKET_HPP

namespace ipc {
	struct packet_start_t {};
	struct packet_end_t {};

	inline constexpr packet_start_t packet_start{};
	inline constexpr packet_end_t packet_end{};
}

#endif   // POPCORN_IDL_LIBIPC_SRC_PACKET_HPP
