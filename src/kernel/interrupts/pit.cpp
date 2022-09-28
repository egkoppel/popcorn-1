/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pit.hpp"

using namespace pit;

pit::Pit timer = Pit();

void Pit::update() {
	auto c = Pit::command_t {
		.bcd = 0,
		.mode = this->mode,
		.access = access_t::LOBYTE_HIBYTE,
		.channel = 0
	};
	command.write(*reinterpret_cast<uint8_t*>(&c));
	channel0_data.write(this->divisor & 0xFF);
	channel0_data.write(this->divisor >> 8);
}

void Pit::set_divisor(uint16_t divisor) {
	this->divisor = divisor;
	this->update();
}

void Pit::set_mode(mode_t mode) {
	this->mode = mode;
	this->update();
}

void Pit::set_frequency(uint16_t frequency) {
	this->set_divisor(1193180 / frequency);
}
