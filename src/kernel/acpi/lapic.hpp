
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_LAPIC_HPP
#define HUG_LAPIC_HPP

namespace lapic_registers {
	// Used to index into u32 pointer so all offsets divided by 4
	enum {
		ID = 0x020 / 4,
		VERSION = 0x030 / 4,
		TASK_PRIORITY = 0x080 / 4,
		ARBITRATION_PRIORITY = 0x090 / 4,
		PROCESSOR_PRIORITY = 0x0A0 / 4,
		EOI = 0x0B0 / 4,
		REMOTE_READ = 0x0C0 / 4,
		LOGICAL_DESTINATION = 0x0D0 / 4,
		DESTINATION_FORMAT = 0x0E0 / 4,
		SPURIOUS_INTERRUPT_VECTOR = 0x0F0 / 4,
		IN_SERVICE = 0x100 / 4,
		TRIGGER_MODE = 0x180 / 4,
		INTERRUPT_REQUEST = 0x200 / 4,
		ERROR_STATUS = 0x280 / 4,
		LVT_CMCI = 0x2F0 / 4,
		ICR_LOW = 0x300 / 4,
		ICR_HIGH = 0x310 / 4,
		LVT_TIMER = 0x320 / 4,
		LVT_THERMAL_SENSOR = 0x330 / 4,
		LVT_PERFORMANCE_MONITORING_COUNTERS = 0x340 / 4,
		LVT_LINT0 = 0x350 / 4,
		LVT_LINT1 = 0x360 / 4,
		LVT_ERROR = 0x370 / 4,
		TIMER_INITIAL_COUNT = 0x380 / 4,
		TIMER_CURRENT_COUNT = 0x390 / 4,
		TIMER_DIVIDE_CONFIGURATION = 0x3E0 / 4,
	};
}

#endif //HUG_LAPIC_HPP
