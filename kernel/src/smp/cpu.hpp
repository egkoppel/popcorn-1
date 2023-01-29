
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_SMP_CPU_HPP
#define POPCORN_KERNEL_SRC_SMP_CPU_HPP

#include <acpi/lapic.hpp>
#include <memory/memory_map.hpp>
#include <popcorn_prelude.h>

class Cpu {
public:
	struct ipi {
		enum destination { DIRECTED = 0, SELF = 1, ALL = 2, ALL_EXCLUDE_SELF = 3 };
		enum delivery_mode { FIXED = 0, LOWEST_PRIORITY = 1, SMI = 2, NMI = 4, INIT = 5, START = 6 };
		enum destination_mode { PHYSICAL = 0, LOGICAL = 1 };
		enum level { ASSERT = 1, DEASSERT = 0 };
		enum trigger_mode { EDGE = 0, LEVEL = 1 };
	};

	Cpu(usize id) : id_(id) {}

	usize id() const { return this->id_; }
	void boot();
	void send_ipi(u8 vector, ipi::delivery_mode, ipi::destination_mode, ipi::level, ipi::trigger_mode);
	static void send_ipi(ipi::destination destinations,
	                     u8 vector,
	                     ipi::delivery_mode,
	                     ipi::destination_mode,
	                     ipi::level,
	                     ipi::trigger_mode);

	static memory::MemoryMap<volatile acpi::lapic_t> lapic;

private:
	usize id_;
	static void send_ipi_internal(u8 destination,
	                              u8 vector,
	                              ipi::delivery_mode,
	                              ipi::destination_mode,
	                              ipi::level,
	                              ipi::trigger_mode,
	                              ipi::destination destinations);
};

extern cpu_local Cpu *local_cpu;

#endif   // POPCORN_KERNEL_SRC_SMP_CPU_HPP
