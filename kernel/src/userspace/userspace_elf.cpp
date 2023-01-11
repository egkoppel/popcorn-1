/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "userspace_elf.hpp"

using namespace Elf64;

constexpr unsigned char ELF_MAGIC[4]            = {0x7F, 'E', 'L', 'F'};
constexpr unsigned char ELF_CLASS_64            = 0x02;
constexpr unsigned char ELF_DATA_LITTLE_ENDIAN  = 0x01;
constexpr unsigned char ELF_VERSION_CURRENT     = 0x01;
constexpr unsigned char ELF_OSABI_UNIX_SYSTEM_V = 0x00;

enum Elf_Type {
	ET_NONE = 0,   // Unkown Type
	ET_REL  = 1,   // Relocatable File
	ET_EXEC = 2    // Executable File
};

constexpr unsigned char ELF_MACHINE_AMD64 = 0x3E;

int Elf64FileHeader::verify_header() {
	if (this->e_ident[0] != ELF_MAGIC[0] || this->e_ident[1] != ELF_MAGIC[1] || this->e_ident[2] != ELF_MAGIC[2]
	    || this->e_ident[3] != ELF_MAGIC[3]) {
		return -1;
	}
	if (this->e_ident[4] != ELF_CLASS_64) { return -1; }
	if (this->e_ident[5] != ELF_DATA_LITTLE_ENDIAN) { return -1; }
	if (this->e_ident[6] != ELF_VERSION_CURRENT) { return -1; }
	if (this->e_ident[7] != ELF_OSABI_UNIX_SYSTEM_V) { return -1; }
	if (this->e_type != ET_EXEC) { return -1; }
	if (this->e_machine != ELF_MACHINE_AMD64) { return -1; }
	return 0;
}
