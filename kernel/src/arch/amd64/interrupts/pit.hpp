/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_PIT_HPP
#define HUGOS_PIT_HPP

#include "port.hpp"

#include <type_traits>

namespace arch::amd64 {
	class Pit {
	private:
		using command_t = struct {
			u8 bcd     : 1;
			u8 mode    : 3;
			u8 access  : 2;
			u8 channel : 2;
		};

		using access_t = enum : u8 { LATCH_COUNT = 0, LOBYTE_ONLY = 1, HIBYTE_ONLY = 2, LOBYTE_HIBYTE = 3 };

		void update() noexcept;

	public:
		using mode_t = enum : u8 { ONESHOT = 1, RATE_GENERATOR = 2, SQUARE_WAVE = 3 };

	private:
		Port<u8> channel0_data{0x40};
		Port<u8> channel1_data{0x41};
		Port<u8> channel2_data{0x42};
		Port<u8> command{0x43};

		u16 divisor = 0;
		mode_t mode      = SQUARE_WAVE;

	public:
		void set_divisor(u16 divisor) noexcept;
		void set_mode(mode_t mode) noexcept;
		void set_frequency(u16 frequency) noexcept;
	};

	extern Pit timer;
}   // namespace arch::amd64

#endif
