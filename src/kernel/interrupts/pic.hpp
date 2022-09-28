/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_PIC_H
#define _HUGOS_PIC_H

#include <../main/port.h>
#include <stdio.h>

namespace pic {
	class Pic {
		private:
		Port<uint8_t> command;
		Port<uint8_t> data;

		public:
		Pic(uint16_t addr): command(addr), data(addr+1) {};
		void EOI();
		uint8_t read_mask();
		void write_data(uint8_t mask);
		void send_command(uint8_t command);
	};

	class ATChainedPIC {
		private:
		Pic master, slave;
		uint8_t offset1, offset2;
		using ports = enum {
			MASTER = 0x0020,
			SLAVE = 0x00A0
		};
		using commands = enum {
			INIT = 0x11,
			EOI = 0x20,
			READ_IRR = 0x0B,
			READ_ISR = 0x0A,
			MODE_8086 = 0x01
		};

		public:
		ATChainedPIC(uint8_t offset1, uint8_t offset2): master(ports::MASTER), slave(ports::SLAVE), offset1(offset1), offset2(offset2) {
			uint8_t master_mask = this->master.read_mask();
			uint8_t slave_mask = this->slave.read_mask();

			printf("Master mask: %b\n", master_mask);
			printf("Slave mask: %b\n", slave_mask);

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
			this->master.write_data(4); // Slave at IRQ2
			io_wait();
			this->slave.write_data(2); // Slave cascade identity
			io_wait();

			// Send mode info
			this->master.write_data(commands::MODE_8086);
			io_wait();
			this->slave.write_data(commands::MODE_8086);
			io_wait();

			// Restore masks
			this->master.write_data(master_mask);
			this->slave.write_data(slave_mask);
		}
		void acknowledge_irq(uint8_t irq_line);
	};
}

#endif