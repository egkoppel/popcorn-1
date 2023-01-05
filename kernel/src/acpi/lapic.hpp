
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
			u64_split(volatile u32& low, volatile u32& high) noexcept : low(&low), high(&high) {}

			explicit(false) operator u64() noexcept {
				return static_cast<u64>(*this->low) | (static_cast<u64>(*this->high) << 32);
			}
			u64_split& operator=(u64 val) noexcept {
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
		alignas(16) u32 eoi_;
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
		alignas(16) u32 lvt_timer;
		alignas(16) u32 lvt_thermal_sensor;
		alignas(16) u32 lvt_perf_monitor;
		alignas(16) u32 lvt_lint0;
		alignas(16) u32 lvt_lint1;
		alignas(16) u32 lvt_error;
		alignas(16) u32 timer_initial_count;
		alignas(16) const u32 timer_current_count;
		const u8 _pad3[64];
		alignas(16) u32 timer_divide_configuration;

		/* TODO when clang supports it
		u256 isr() { return ... }
		u256 tmr() { return ... }
		u256 irr() { return ... }
		*/
		details::u64_split interrupt_command_register() volatile noexcept {
			return {this->interrupt_command_low, this->interrupt_command_high};
		}

		enum timer_mode : unsigned _BitInt(2) { ONE_SHOT = 0, PERIODIC = 1, TSC_DEADLINE = 2 };
		enum timer_divisor : unsigned _BitInt(4) {
			DIV1   = 0b1011,
			DIV2   = 0b0000,
			DIV4   = 0b0001,
			DIV8   = 0b0010,
			DIV16  = 0b0011,
			DIV32  = 0b1000,
			DIV64  = 0b1001,
			DIV128 = 0b1010
		};

		void configure_timer(u8 vector, timer_mode mode, timer_divisor divisor) volatile noexcept {
			this->timer_divide_configuration = divisor;
			this->lvt_timer                  = static_cast<u32>(vector) | (static_cast<u32>(mode) << 17);
			this->lvt_timer &= ~(1 << 16);   // Unmask the timer
		}

		void eoi() volatile noexcept { this->eoi_ = 0; }
	};

	static_assert(offsetof(lapic, id) == 0x020);
	static_assert(offsetof(lapic, version) == 0x030);
	static_assert(offsetof(lapic, task_priority) == 0x080);
	static_assert(offsetof(lapic, arbitration_priority) == 0x090);
	static_assert(offsetof(lapic, processor_priority) == 0x0A0);
	static_assert(offsetof(lapic, eoi_) == 0x0B0);
	static_assert(offsetof(lapic, remote_read) == 0x0C0);
	static_assert(offsetof(lapic, logical_destination) == 0x0D0);
	static_assert(offsetof(lapic, destination_format) == 0x0E0);
	static_assert(offsetof(lapic, spurious_interrupt_vector) == 0x0F0);
	static_assert(offsetof(lapic, error_status) == 0x280);
	static_assert(offsetof(lapic, cmci) == 0x2F0);
	static_assert(offsetof(lapic, interrupt_command_low) == 0x300);
	static_assert(offsetof(lapic, interrupt_command_high) == 0x310);
	static_assert(offsetof(lapic, timer_divide_configuration) == 0x3e0);
}   // namespace acpi

#endif   // HUGOS_LAPIC_HPP
