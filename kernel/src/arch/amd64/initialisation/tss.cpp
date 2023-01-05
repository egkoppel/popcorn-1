/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tss.hpp"

#include "gdt.hpp"

using namespace arch::amd64;

TSS::TSS() noexcept {
	this->_res0 = 0;
	this->_res1 = 0;
	this->_res2 = 0;
	this->_res3 = 0;
	for (auto& i : this->interrupt_stack_table) { i = memory::vaddr_t{.address = 0}; }
	for (auto& i : this->privilege_stack_table) { i = memory::vaddr_t{.address = 0}; }
}

void TSS::load(uint16_t gdt_index) noexcept { __asm__ volatile("ltr %w0" : : "q"(gdt_index * 8)); }

void TSS::add_stack(uint8_t stack_idx, const memory::KStack<>& stack) noexcept {
	this->interrupt_stack_table[stack_idx] = *stack.top();
}

TSS arch::amd64::task_state_segment = TSS();
