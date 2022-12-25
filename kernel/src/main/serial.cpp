/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arch/amd64/interrupts/port.hpp>
#include <stdexcept>
#include <stdint.h>

class SerialPort {
private:
	Port<uint8_t> data, interupt_enable, fifo_control, line_control, modem_contol, line_status, modem_status, scratch;

public:
	explicit SerialPort(uint16_t port) :
		data(port),
		interupt_enable(port + 1),
		fifo_control(port + 2),
		line_control(port + 3),
		modem_contol(port + 4),
		line_status(port + 5),
		modem_status(port + 6),
		scratch(port + 7) {
		this->interupt_enable.write(0);   // Disable interrupts
		this->line_control.write(0x80);   // Set DLAB bit (maps first two ports to baud divisor)
		this->data.write(3);              // Set divisor to 3 (38400 baud)
		this->interupt_enable.write(0);
		this->line_control.write(0x03);   // 8 bits, no parity, one stop bit
		this->fifo_control.write(0xC7);   // Enable FIFO, clear them, with 14-byte threshold
		this->modem_contol.write(0x1E);   // Set to loopback mode

		this->data.write(0xAE);   // Write to port
		if (this->data.read() != 0xAE) { throw std::runtime_error("Serial port failure"); }

		this->modem_contol.write(0x0F);   // Turn off loopback, enable IRQs
	}

	int is_transmit_empty() const noexcept { return this->line_status.read() & 0x20; }
	void write(char a) noexcept {
		while (is_transmit_empty() == 0)
			;
		this->data.write(a);
	};
	void print(char *s) noexcept {
		while (*s) {
			this->write(*s);
			s++;
		}
	}
};

SerialPort serial1(0x3f8);

extern "C" {
	int is_transmit_empty() { return serial1.is_transmit_empty(); }

	void write_serial(char a) { serial1.write(a); }

	void print_serial(char *a) { serial1.print(a); }
}