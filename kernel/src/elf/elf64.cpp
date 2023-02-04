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

	[[noreturn]] void dyld_terminate(const char *message) {
		LOG(Log::CRITICAL, "Unable to load executable: %s", message);
		while (true) threads::local_scheduler->block_task(threads::Task::State::KILLED);
	}

	/*[[noreturn]] void dyld_terminate(const std::string& message) {
	    dyld_terminate(message.c_str());
	}*/

	void exec(std::byte *file) noexcept {
		auto header          = reinterpret_cast<file_header *>(file);
		auto program_headers = reinterpret_cast<program_header *>(file + header->program_header_offset);
		for (usize i = 0; i < header->program_header_entry_count; ++i) {
			const auto& segment = program_headers[i];
			LOG(Log::INFO,
			    "Found header of type %s",
			    segment.type == segment_type::PT_NULL    ? "NULL" :
			    segment.type == segment_type::PT_LOAD    ? "LOAD" :
			    segment.type == segment_type::PT_INTERP  ? "INTERP" :
			    segment.type == segment_type::PT_DYNAMIC ? "DYNAMIC" :
			                                               "(unsupported)");

			switch (segment.type) {
				case segment_type::PT_INTERP: {
					auto interpreter = reinterpret_cast<const char *>(file + segment.file_offset);
					if (std::strcmp(interpreter, "dyld.exec") != 0) {
						dyld_terminate(("Unknown interpreter"s + interpreter).c_str());
					}
					break;
				}
				case segment_type::PT_LOAD: {
					auto aligned_start = memory::aligned<memory::vaddr_t>::aligned_down(segment.load_addr);
					for (auto offset = 0uz; offset < segment.size_in_memory; offset += 0x1000) {
						try {
							auto _ = threads::local_scheduler->get_current_task()
							                             ->new_mmap(aligned_start.address + offset, 0x1000, false, true);
						} catch (std::bad_alloc&) { dyld_terminate("Couldn't load segment at required address"); }
					}
					std::memcpy(static_cast<void *>(segment.load_addr),
					            file + segment.file_offset,
					            segment.size_on_disk);
					std::memset(static_cast<void *>(segment.load_addr + segment.size_on_disk),
					            0,
					            segment.size_in_memory - segment.size_on_disk);
					break;
				}
				default: break;
			}
		}
		auto entrypoint = static_cast<void (*)()>(header->entrypoint);
		entrypoint();
		while (true) threads::local_scheduler->yield();
	}
}   // namespace Elf64
