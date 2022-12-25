
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MACROS_HPP
#define HUGOS_MACROS_HPP

#include <stdint.h>

extern inline void wrsmr(uint32_t reg, uint64_t val) {
	__asm__ volatile("wrmsr" : : "d"((val) >> 32), "a"(val & 0xFFFFFFFF), "c"(reg));
}

#endif //HUGOS_MACROS_HPP
