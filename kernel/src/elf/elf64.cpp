/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "elf64.hpp"

#include <cstddef>
#include <memory/types.hpp>
#include <string>
#include <threading/scheduler.hpp>

using namespace std::literals;

namespace Elf64 {
	enum class file_class : u8 { bits_32 = 1, bits_64 = 2 };
	enum class file_endian : u8 { little = 1, big = 2 };
	enum class file_abi : u8 { system_v = 0, hp_ux = 1, embedded = 255 };
	enum class file_type : u16 {};

	struct [[gnu::packed]] file_header {
		char magic[4];
		file_class class_;
		file_endian endianness;
		u8 version;
		file_abi abi;
		u8 abi_version;
		u8 _padding[7];
		file_type type;
		u16 machine_type;
		u32 object_file_version;
		memory::vaddr_t entrypoint;
		u64 program_header_offset;
		u64 section_header_offset;
		u32 processor_flags;
		u16 header_size;
		u16 program_header_entry_size;
		u16 program_header_entry_count;
		u16 section_header_entry_size;
		u16 section_header_entry_count;
		u16 string_section_idx;
	};

	struct [[gnu::packed]] program_header {
		segment_type type;
		segment_flags flags;
		u64 file_offset;
		memory::vaddr_t load_addr;
		memory::paddr_t reserved;
		u64 size_on_disk;
		u64 size_in_memory;
		u64 alignment;
	};
}   // namespace Elf64
