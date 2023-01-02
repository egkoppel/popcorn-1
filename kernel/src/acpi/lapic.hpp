
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_LAPIC_HPP
#define HUGOS_LAPIC_HPP

#include <cstddef>
#include <popcorn_prelude.h>

namespace acpi {
	namespace details {
		class u64_split {
		public:
			u64_split(volatile u32& low, volatile u32& high) : low(&low), high(&high) {}

			explicit(false) operator u64() {
				return static_cast<u64>(*this->low) | (static_cast<u64>(*this->high) << 32);
			}
			u64_split& operator=(u64 val) {
				// MUST WRITE TO LOW LAST BECAUSE THAT TRIGGERS THE IPI TO SEND
				*this->high = (val >> 32) & 0xFFFF;
				*this->low  = val & 0xFFFF;
				return *this;
			}

		private:
			volatile u32 *low;
			volatile u32 *high;
		};
	}   // namespace details

	struct [[gnu::packed]] lapic {
		const u8 _pad0[0x020];
		alignas(16) u32 id;
		alignas(16) const u32 version;
		const u8 _pad1[76];
		alignas(16) u32 task_priority;
		alignas(16) const u32 arbitration_priority;
		alignas(16) const u32 processor_priority;
		alignas(16) u32 eoi;
		alignas(16) const u32 remote_read;
		alignas(16) u32 logical_destination;
		alignas(16) u32 destination_format;
		alignas(16) u32 spurious_interrupt_vector;
		alignas(16) const u32 isr0;
		alignas(16) const u32 isr1;
		alignas(16) const u32 isr2;
		alignas(16) const u32 isr3;
		alignas(16) const u32 isr4;
		alignas(16) const u32 isr5;
		alignas(16) const u32 isr6;
		alignas(16) const u32 isr7;
		alignas(16) const u32 tmr0;
		alignas(16) const u32 tmr1;
		alignas(16) const u32 tmr2;
		alignas(16) const u32 tmr3;
		alignas(16) const u32 tmr4;
		alignas(16) const u32 tmr5;
		alignas(16) const u32 tmr6;
		alignas(16) const u32 tmr7;
		alignas(16) const u32 irr0;
		alignas(16) const u32 irr1;
		alignas(16) const u32 irr2;
		alignas(16) const u32 irr3;
		alignas(16) const u32 irr4;
		alignas(16) const u32 irr5;
		alignas(16) const u32 irr6;
		alignas(16) const u32 irr7;
		alignas(16) u32 error_status;
		const u8 _pad2[96];
		alignas(16) u32 cmci;
		alignas(16) u32 interrupt_command_low;
		alignas(16) u32 interrupt_command_high;

		/* TODO when clang supports it
		u256 isr() { return ... }
		u256 tmr() { return ... }
		u256 irr() { return ... }
		*/
		details::u64_split interrupt_command_register() volatile;
	};
	static_assert(offsetof(lapic, id) == 0x020);
	static_assert(offsetof(lapic, version) == 0x030);
	static_assert(offsetof(lapic, task_priority) == 0x080);
	static_assert(offsetof(lapic, arbitration_priority) == 0x090);
	static_assert(offsetof(lapic, processor_priority) == 0x0A0);
	static_assert(offsetof(lapic, eoi) == 0x0B0);
	static_assert(offsetof(lapic, remote_read) == 0x0C0);
	static_assert(offsetof(lapic, logical_destination) == 0x0D0);
	static_assert(offsetof(lapic, destination_format) == 0x0E0);
	static_assert(offsetof(lapic, spurious_interrupt_vector) == 0x0F0);
	static_assert(offsetof(lapic, error_status) == 0x280);
	static_assert(offsetof(lapic, cmci) == 0x2F0);
	static_assert(offsetof(lapic, interrupt_command_low) == 0x300);
	static_assert(offsetof(lapic, interrupt_command_high) == 0x310);
}   // namespace acpi

#endif   // HUGOS_LAPIC_HPP
