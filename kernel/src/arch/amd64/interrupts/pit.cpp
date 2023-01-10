/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pit.hpp"

#include <bit>

namespace arch::amd64 {
	Pit timer = Pit();

	void Pit::update() noexcept {
		Pit::command_t c{.bcd = 0, .mode = this->mode, .access = access_t::LOBYTE_HIBYTE, .channel = 0};
		command.write(std::bit_cast<u8>(c));
		channel0_data.write(this->divisor & 0xFF);
		channel0_data.write(this->divisor >> 8);
	}

	void Pit::set_divisor(u16 divisor) noexcept {
		this->divisor = divisor;
		this->update();
	}

	void Pit::set_mode(mode_t mode) noexcept {
		this->mode = mode;
		this->update();
	}

	void Pit::set_frequency(u16 frequency) noexcept {
		this->set_divisor(1193180 / frequency);
	}
}   // namespace arch::amd64
