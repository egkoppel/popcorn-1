/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pic.hpp"

#include "port.hpp"

#include <log.hpp>

namespace arch::amd64 {
	namespace commands {
		enum commands { INIT = 0x11, EOI = 0x20, READ_IRR = 0x0B, READ_ISR = 0x0A, MODE_8086 = 0x01 };
	}

	void Pic::EOI() noexcept {
		this->send_command(commands::EOI);
	}

	uint8_t Pic::read_mask() noexcept {
		return this->data.read();
	}

	void Pic::write_data(u8 mask) noexcept {
		this->data.write(mask);
	}

	void Pic::send_command(u8 command) noexcept {
		this->command.write(command);
	}

	void ATChainedPIC::acknowledge_irq(u8 irq_line) noexcept {
		if (irq_line >= this->offset1 && irq_line < this->offset1 + 8) {
			this->master.EOI();
		} else if (irq_line >= this->offset2 && irq_line < this->offset2 + 8) {
			this->master.EOI();
			this->slave.EOI();
		}
	}

	ATChainedPIC::ATChainedPIC(u8 offset1, u8 offset2) noexcept
		: master(ports::MASTER),
		  slave(ports::SLAVE),
		  offset1(offset1),
		  offset2(offset2) {
		// Start init sequence
		this->master.send_command(commands::INIT);
		io_wait();
		this->slave.send_command(commands::INIT);
		io_wait();

		// Send offsets to master and slave PICs
		this->master.write_data(offset1);
		io_wait();
		this->slave.write_data(offset2);
		io_wait();

		// Send wiring info
		this->master.write_data(4);   // Slave at IRQ2
		io_wait();
		this->slave.write_data(2);   // Slave cascade identity
		io_wait();

		// Send mode info
		this->master.write_data(commands::MODE_8086);
		io_wait();
		this->slave.write_data(commands::MODE_8086);
		io_wait();

		// Mask all interrupts
		this->master.write_data(0xFF);
		this->slave.write_data(0xFF);
	}

	ATChainedPIC pics(0x20, 0x28);
}   // namespace arch::amd64
