/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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
	regular = '0',
	hardlink = '1',
	symlink = '2',
	character_device = '3',
	block_device = '4',
	directory = '5',
	fifo = '6'
};

struct __attribute__((packed)) tar_file_header {
	char filename[100];
	uint64_t mode;
	uint64_t uid;
	uint64_t gid;
	char size[12];
	char mtime[12];
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

int oct2bin(char *str, int size) {
	int n = 0;
	char *c = str;
	while (size-- > 0) {
		n *= 8;
		n += *c - '0';
		c++;
	}
	return n;
}

size_t Initramfs::locate_file(const char *filename, void **data) {
	auto *ptr = reinterpret_cast<tar_file_header *>(this->data_start);

	while ((uint64_t)ptr < this->data_end && memcmp(ptr->ustar, "ustar", 5) == 0) {
		int filesize = oct2bin(ptr->size, 11);
		if (strcmp(ptr->filename + 10, filename) == 0) { /* ptr->filename + 10 gives filename without preceding initramfs/ */
			*data = static_cast<void *>(ADD_BYTES(ptr, 512));
			return filesize;
		}
		ptr = ADD_BYTES(ptr, (((filesize + 511) / 512) + 1) * 512);
	}

	return 0;
}
