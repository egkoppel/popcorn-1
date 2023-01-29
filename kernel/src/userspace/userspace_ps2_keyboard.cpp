
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "userspace_ps2_keyboard.hpp"

#include "userspace_macros.hpp"

#include <tuple>

namespace driver::ps2_keyboard {
	enum class commands : u8 {
		port2_disable   = 0xa7,
		port2_enable    = 0xa8,
		port2_test      = 0xa9,
		controller_test = 0xaa,
		port1_test      = 0xab,
		port1_disable   = 0xad,
		port1_enable    = 0xae,
		port2_send      = 0xd4,
		read_config     = 0x20,
		write_config    = 0x60
	};

	enum status {
		port1_irq_enable         = 1 << 0,
		port2_irq_enable         = 1 << 1,
		port1_clock_disable      = 1 << 4,
		port2_clock_disable      = 1 << 5,
		port1_translation_enable = 1 << 6
	};

	enum config { read_available = 1 << 0, write_full = 1 << 1 };

	constexpr u16 reg_data    = 0x60;
	constexpr u16 reg_status  = 0x64;
	constexpr u16 reg_command = 0x64;

	namespace {
		inline void outb(u16 port, u8 val) noexcept {
			__asm__ volatile("outb %b0, %1" : : "r"(val), "Nd"(port));
		}
		inline u8 inb(u16 port) {
			u8 ret;
			__asm__ volatile("inb %1, %b0" : "=r"(ret) : "Nd"(port));
			return ret;
		}

		void send_command(commands command) {
			outb(reg_command, static_cast<u8>(command));
		}

		u8 get_response() {
			while (auto i = !(inb(reg_status) & read_available)) {
				__asm__ volatile("" : "+g"(i));   // Enforce dependency to prevent optimizing away
			}
			return inb(reg_data);
		}

		void send_data(u8 data) {
			while (auto i = (inb(reg_status) & write_full)) {
				__asm__ volatile("" : "+g"(i));   // Enforce dependency to prevent optimizing away
			}
			outb(reg_data, data);
		}

		std::tuple<bool, bool> init() {
			bool port2_working = true;

			send_command(commands::port1_disable);
			send_command(commands::port2_disable);

			// clear buffer
			while (inb(reg_status) & read_available) { inb(reg_data); }

			send_command(commands::read_config);
			auto config_byte = get_response();
			config_byte &= ~(port1_irq_enable | port2_irq_enable | port1_translation_enable);   // disable irqs and
			                                                                                    // scancode translation
			send_command(commands::write_config);
			send_data(config_byte);

			if ((config_byte & port2_clock_disable) == 0) port2_working = false;

			send_command(commands::controller_test);
			if (get_response() != 0x55) {
				LOG(Log::WARNING, "PS/2 controller failed self-test");
				return std::make_tuple(false, false);
			} else {
				LOG(Log::INFO, "PS/2 controller passed self-test");
			}

			// reset config since self-test sometimes causes reset
			send_command(commands::write_config);
			send_data(config_byte);

			if (port2_working) {
				// Make sure it's actually dual port
				send_command(commands::port2_enable);
				send_command(commands::read_config);
				config_byte = get_response();
				if ((config_byte & port2_clock_disable) != 0) port2_working = false;
				send_command(commands::port2_disable);
			}

			LOG(Log::INFO, "PS/2 dual port? %d", port2_working);

			bool port1_working = true;

			send_command(commands::port1_test);
			if (auto error_code = get_response()) {
				LOG(Log::WARNING, "Port 1 failed self-test with code 0x%x", error_code);
				port1_working = false;
			}

			if (port2_working) {
				send_command(commands::port2_test);
				if (auto error_code = get_response()) {
					LOG(Log::WARNING, "Port 2 failed self-test with code 0x%x", error_code);
					port2_working = false;
				}
			}

			return std::make_tuple(port1_working, port2_working);
		}

		bool send_command(int ps2_port, u8 command) {
			if (ps2_port == 2) { send_command(commands::port2_send); }
			send_data(command);
			if (get_response() == 0xFA) return false;
			else return true;
		}
	}   // namespace

	int main() {
		/* TODO: check FADT for existence of PS/2 */
		auto [port1, port2] = init();

		if (!port1 && !port2) {
			LOG(Log::WARNING, "PS/2 failure");
			_syscall_new(SyscallVectors::suspend);
			while (true) __asm__ volatile("nop");
		}

		if (port1) send_command(commands::port1_enable);
		if (port2) send_command(commands::port2_enable);

		if (port1) {
			auto b = send_command(1, 0xFF);   // reset
			if (get_response() == 0xFC) port1 = false;
		}

		if (port2) {
			auto b = send_command(2, 0xFF);   // reset
			if (get_response() == 0xFC) port2 = false;
		}

		if (port1) {
			auto b     = send_command(1, 0xF5);   // disable scanning
			b          = send_command(1, 0xF2);   // identify
			auto byte1 = get_response();
			u8 byte2   = 0;
			if (byte1 == 0xAB || byte1 == 0xAC) byte2 = get_response();
			b = send_command(1, 0xF4);   // enable scanning
			LOG(Log::INFO,
			    "Port 1 is 0x%x 0x%x - %s",
			    byte1,
			    byte2,
			    (byte1 == 0 || byte1 == 3 || byte1 == 4) ? "mouse" : "probs keyboard");
		}

		if (port2) {
			auto b     = send_command(2, 0xF5);   // disable scanning
			b          = send_command(2, 0xF2);   // identify
			auto byte1 = get_response();
			u8 byte2   = 0;
			if (byte1 == 0xAB || byte1 == 0xAC) byte2 = get_response();
			b = send_command(2, 0xF4);   // enable scanning
			LOG(Log::INFO,
			    "Port 2 is 0x%x 0x%x - %s",
			    byte1,
			    byte2,
			    (byte1 == 0 || byte1 == 3 || byte1 == 4) ? "mouse" : "probs keyboard");
		}

		auto err = _syscall_new(SyscallVectors::register_isa_irq, 0x1);
		if (err < 0) {
			LOG(Log::CRITICAL, "failed to register keyboard irq");
			while (true) __asm__ volatile("nop");
		}
		while (true) {
			_syscall_new(SyscallVectors::suspend);
			LOG(Log::WARNING, "keyboard irq");
			inb(0x60);
		}
	}
}   // namespace driver::ps2_keyboard
