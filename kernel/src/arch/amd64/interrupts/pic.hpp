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

#include <popcorn_prelude.h>
#include <stdio.h>

namespace arch::amd64 {
	class Pic {
	private:
		Port<u8> command;
		Port<u8> data;

	public:
		explicit Pic(u16 addr) noexcept : command(addr), data(addr + 1){};
		void EOI() noexcept;
		uint8_t read_mask() noexcept;
		void write_data(u8 mask) noexcept;
		void send_command(u8 command) noexcept;
	};

	class ATChainedPIC {
	private:
		Pic master, slave;
		u8 offset1, offset2;
		using ports    = enum { MASTER = 0x0020, SLAVE = 0x00A0 };
		using commands = enum { INIT = 0x11, EOI = 0x20, READ_IRR = 0x0B, READ_ISR = 0x0A, MODE_8086 = 0x01 };

	public:
		ATChainedPIC(u8 offset1, u8 offset2) noexcept;
		void acknowledge_irq(u8 irq_line) noexcept;
	};

	extern ATChainedPIC pics;
}   // namespace arch::amd64

#endif   //HUGOS_PIC_HPP
