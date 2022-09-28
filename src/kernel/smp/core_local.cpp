/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core_local.hpp"
#include <stdint.h>
#include <vector>

constexpr uint64_t FS_BASE_MSR = 0xC0000100;
constexpr uint64_t GS_BASE_MSR = 0xC0000101;
constexpr uint64_t GS_KERNEL_BASE_MSR = 0xC0000102;

std::vector<core_local *> core_local_data;
atomic_uint_fast64_t core_local::next_idx = 0;

core_local::core_local() {
	__asm__ volatile("wrmsr" : : "a"((uint32_t)(uint64_t)this), "d"((uint32_t)(((uint64_t)this) >> 32)), "c"(GS_BASE_MSR));
	core_local_data.push_back(this);
}