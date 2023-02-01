/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "initramfs.hpp"

#include <stdint.h>
#include <string.h>
#include <utils.h>

enum class file_type : uint8_t {
	regular          = '0',
	hardlink         = '1',
	symlink          = '2',
	character_device = '3',
	block_device     = '4',
	directory        = '5',
	fifo             = '6'
};

struct [[gnu::packed]] tar_file_header {
	char filename[100];
	uint64_t mode;
	uint64_t uid;
	uint64_t gid;
	char size[12];
	[[maybe_unused]] char mtime[12];
	uint64_t checksum;
	file_type type;
	char linked_filename[100];
	char ustar[6];
	char version[2];
	char user_name[32];
	char group_name[32];
	uint64_t device_major;
	uint64_t device_minor;
	char filename_prefix[155];
};

usize oct2bin(char *str, int size) {
	usize n = 0;
	char *c = str;
	while (size-- > 0) {
		n *= 8;
		n += *c - '0';
		c++;
	}
	return n;
}

Initramfs::File Initramfs::get_file(const char *filename) {
	auto ptr = reinterpret_cast<tar_file_header *>(this->data.get());

	while (true) {
		LOG(Log::DEBUG, "ramfs found file: %s", ptr->filename);
		LOG(Log::DEBUG,
		    "ustar check: %c%c%c%c%c (%d)",
		    ptr->ustar[0],
		    ptr->ustar[1],
		    ptr->ustar[2],
		    ptr->ustar[3],
		    ptr->ustar[4],
		    strncmp(ptr->ustar, "ustar", 5));
		LOG(Log::DEBUG, "Current ptr: %lp - search in %lp -> %lp", ptr, this->data.start(), this->data.end());

		if (!(memory::vaddr_t{.address = reinterpret_cast<usize>(ptr)} < this->data.end()
		      && strncmp(ptr->ustar, "ustar", 5) == 0))
			break;
		usize filesize = oct2bin(ptr->size, 11);
		if (strcmp(ptr->filename + 10, filename) == 0) { /* ptr->filename + 10 gives filename without preceding
			                                                initramfs/ */
			return {reinterpret_cast<std::byte *>(ADD_BYTES(ptr, 512)), filesize};
		}
		ptr = ADD_BYTES(ptr, (((filesize + 511) / 512) + 1) * 512);
	}

	throw FileNotFoundException(filename);
}

memory::physical_allocators::NullAllocator Initramfs::alloc;
