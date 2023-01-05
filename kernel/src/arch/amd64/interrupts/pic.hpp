/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_PIC_HPP
#define HUGOS_PIC_HPP

#include "port.hpp"

#include <stdio.h>

namespace arch::amd64 {
	class Pic {
	private:
		Port<uint8_t> command;
		Port<uint8_t> data;

	public:
		explicit Pic(uint16_t addr) noexcept : command(addr), data(addr + 1){};
		void EOI() noexcept;
		uint8_t read_mask() noexcept;
		void write_data(uint8_t mask) noexcept;
		void send_command(uint8_t command) noexcept;
	};

	class ATChainedPIC {
	private:
		Pic master, slave;
		uint8_t offset1, offset2;
		using ports    = enum { MASTER = 0x0020, SLAVE = 0x00A0 };
		using commands = enum { INIT = 0x11, EOI = 0x20, READ_IRR = 0x0B, READ_ISR = 0x0A, MODE_8086 = 0x01 };

	public:
		ATChainedPIC(uint8_t offset1, uint8_t offset2) noexcept;
		void acknowledge_irq(uint8_t irq_line) noexcept;
	};

	extern ATChainedPIC pics;
}   // namespace arch::amd64

#endif   //HUGOS_PIC_HPP