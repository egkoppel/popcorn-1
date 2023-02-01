/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_INITRAMFS_H
#define _HUGOS_INITRAMFS_H

#include <cstddef>
#include <cstdint>
#include <memory/memory_map.hpp>
#include <memory/physical_allocators/null_allocator.hpp>
#include <popcorn_prelude.h>
#include <stdexcept>

using namespace std::literals;

class Initramfs {
private:
	static constexpr auto flags = memory::paging::PageTableFlags::NO_EXECUTE;

public:
	class File {
	public:
		File(std::byte *data, usize file_size) : data_(data), file_size(file_size) {}


		usize size() const { return this->file_size; }
		std::byte *data() { return this->data_; }
		const std::byte *data() const { return this->data_; }

	private:
		std::byte *data_;
		usize file_size;
	};

	class FileNotFoundException : public std::runtime_error {
	public:
		explicit FileNotFoundException(const char *filename) : std::runtime_error("File not found: "s + filename) {}
	};

	Initramfs(memory::paddr_t data_start, usize size) : data(data_start, size, flags, alloc, memory::paging::kas){};
	File get_file(const char *filename);


private:
	memory::MemoryMap<std::byte> data;

	static memory::physical_allocators::NullAllocator alloc;
};

#endif
