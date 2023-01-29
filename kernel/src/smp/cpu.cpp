
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cpu.hpp"

memory::MemoryMap<volatile acpi::lapic_t> Cpu::lapic{};
cpu_local Cpu *local_cpu = nullptr;

void Cpu::send_ipi(u8 vector,
                   ipi::delivery_mode delivery_mode,
                   ipi::destination_mode destination_mode,
                   ipi::level level,
                   ipi::trigger_mode trigger_mode) {
	Cpu::send_ipi_internal(this->id(), vector, delivery_mode, destination_mode, level, trigger_mode, ipi::DIRECTED);
}

void Cpu::send_ipi(ipi::destination destinations,
                   u8 vector,
                   ipi::delivery_mode delivery_mode,
                   ipi::destination_mode destination_mode,
                   ipi::level level,
                   ipi::trigger_mode trigger_mode) {
	Cpu::send_ipi_internal(0, vector, delivery_mode, destination_mode, level, trigger_mode, destinations);
}

void Cpu::send_ipi_internal(u8 destination,
                            u8 vector,
                            ipi::delivery_mode delivery_mode,
                            ipi::destination_mode destination_mode,
                            ipi::level level,
                            ipi::trigger_mode trigger_mode,
                            ipi::destination destinations) {
	lapic->error_status                 = 0;
	lapic->interrupt_command_register() = (static_cast<u64>(destination) << 56)
	                                      | ((static_cast<u64>(destinations) & 0b11) << 18)
	                                      | ((static_cast<u64>(trigger_mode) & 0b1) << 15)
	                                      | ((static_cast<u64>(level) & 0b1) << 14)
	                                      | ((static_cast<u64>(destination_mode) & 0b1) << 11)
	                                      | ((static_cast<u64>(delivery_mode) & 0b111) << 8) | static_cast<u64>(vector);
	do { __asm__ volatile("pause" ::: "memory"); } while (lapic->interrupt_command_register() & (1 << 12));
}

void Cpu::boot() {}
