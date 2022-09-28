/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tss.hpp"

using namespace tss;

TSS::TSS() {
	this->_res0 = 0;
	this->_res1 = 0;
	this->_res2 = 0;
	this->_res3 = 0;
	for (auto& i : this->interrupt_stack_table) {
		i = 0;
	}
	for (auto& i : this->privilege_stack_table) {
		i = 0;
	}
}

void TSS::load(uint16_t gdt_index) {
	asm volatile("ltr %w0" : : "q" (gdt_index*8));
}
