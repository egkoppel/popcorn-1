
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_BOOT_MODULE_HPP
#define HUGOS_BOOT_MODULE_HPP

#include "multiboot.hpp"

namespace multiboot::tags {
	class [[gnu::packed]] BootModule : public Tag {
	private:
		memory::paddr32_t module_start;
		memory::paddr32_t module_end;
		char str;

	public:
		BootModule(const BootModule&) = delete;

		inline const char *name() { return &this->str; }
		inline memory::paddr_t begin() const { return this->module_start; }
		inline memory::paddr_t end() const { return this->module_end; }
		inline u64 module_size() const { return this->end().address - this->begin().address; }
	};
}   // namespace multiboot::tags

#endif   //HUGOS_BOOT_MODULE_HPP
