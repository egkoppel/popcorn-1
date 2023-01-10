
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HUGOS_MACROS_HPP
#define HUGOS_MACROS_HPP

#include <cstdint>
#include <memory/types.hpp>
#include <popcorn_prelude.h>

extern inline void wrsmr(u32 reg, u64 val) {
	__asm__ volatile("wrmsr" : : "d"((val) >> 32), "a"(val & 0xFFFFFFFF), "c"(reg));
}

extern inline memory::paddr_t get_current_page_table_addr() {
	usize ret;
	__asm__ volatile("mov %%cr3, %0" : "=r"(ret));
	return {.address = ret};
}

#endif   //HUGOS_MACROS_HPP
