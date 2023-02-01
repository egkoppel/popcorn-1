/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fsd.hpp"

#include "../memory/vm_map.hpp"
#include "initramfs.hpp"
#include "userspace_macros.hpp"

#include <stdlib.h>

// Initramfs initramfs(0, 0);

struct fd_t {
	char *start_addr;
	char *end_addr;
	char *read_addr;
};

fd_t file_descriptors[16];
uint64_t next_fd = 0;

static char *pointer2 = (char *)0x800000;
static char *pointer  = (char *)0xfc000000;

static inline fsd_command_response_t fsd_mount(fsd_command_t *command_) {
	auto command = reinterpret_cast<fsd_command_mount_t *>(&command_->data);

	if (command->mountpoint != 'a' && command->mountpoint != 'A') return fsd_command_response_t{.return_code = -1};

	if (strncmp("initramfs", command->driver_info, 9) == 0) {
		command->driver_info[command->driver_command_len] = '\0';
		const char *s                                     = command->driver_info;
		while (*s++ != ' ') {};
		auto initramfs_addr_ = atoll_p(&s);
		auto initramfs_addr  = *reinterpret_cast<uint64_t *>(&initramfs_addr_);
		while (*s++ != ' ') {};
		auto initramfs_size_ = atoll_p(&s);
		auto initramfs_size  = *reinterpret_cast<uint64_t *>(&initramfs_size_);

		// initramfs = Initramfs(initramfs_addr, initramfs_addr + initramfs_size);

		return fsd_command_response_t{.return_code = 0};
	} else {
		return fsd_command_response_t{.return_code = -2};
	}
}
static inline fsd_command_response_t fsd_open(fsd_command_t *command_) {
	auto command = reinterpret_cast<fsd_command_open_t *>(&command_->data);

	if (command->path_len >= 248) return fsd_command_response_t{.return_code = -2};
	if (next_fd >= 16) return fsd_command_response_t{.return_code = -1};
	if (strncmp("initramfs:/", command->path, 11) != 0) return fsd_command_response_t{.return_code = -3};

	command->path[command->path_len] = '\0';
	void *file_addr;
	size_t file_size = 0;   // initramfs.locate_file(command->path + 11, &file_addr); /* +11 to remove mountpoint name
	                        // */
	if (file_size == 0) return fsd_command_response_t{.return_code = -4};

	uint64_t fd          = next_fd++;
	file_descriptors[fd] = fd_t{.start_addr = (char *)file_addr,
	                            .end_addr   = (char *)file_addr + file_size,
	                            .read_addr  = (char *)file_addr};

	return fsd_command_response_t{.return_code = 0};
}
static inline fsd_command_response_t fsd_read(fsd_command_t *command_) {
	auto command = reinterpret_cast<fsd_command_read_t *>(&command_->data);

	if (command->fd >= 16) return fsd_command_response_t{.return_code = -1};
	if (file_descriptors[command->fd].start_addr == nullptr) return fsd_command_response_t{.return_code = -2};

	// TODO
	/*auto file_vm_region_handle =
	        vm_region_new_anon(command->size, VmMapping::PROT_READ | VmMapping::PROT_WRITE, VmMapping::PROT_READ);
	if (!file_vm_region_handle) return fsd_command_response_t{.return_code = -3};

	auto res = vm_map_region(pointer, file_vm_region_handle, VmMapping::PROT_READ | VmMapping::PROT_WRITE);
	if (res) return fsd_command_response_t{.return_code = -4};

	if (file_descriptors[command->fd].read_addr + command->size > file_descriptors[command->fd].end_addr)
	    command->size = file_descriptors[command->fd].end_addr - file_descriptors[command->fd].read_addr;

	memcpy(pointer, file_descriptors[command->fd].read_addr, command->size);
	file_descriptors[command->fd].read_addr += command->size;
	pointer = ALIGN_UP(pointer + command->size, 0x1000);

	vm_map_region(pointer2, file_vm_region_handle, VmMapping::PROT_READ | VmMapping::PROT_WRITE);
	*pointer2 = '3';*/

	return fsd_command_response_t{.return_code = -1};   // 0, .data = {.read = {.mmap_vm_handle =
	                                                    // file_vm_region_handle}}};
}

static inline fsd_command_response_t process_command(fsd_command_t& command) {
	switch (command.command) {
		case fsd_command_t::MOUNT: return fsd_mount(&command);
		case fsd_command_t::OPEN: return fsd_open(&command);
		case fsd_command_t::READ: return fsd_read(&command);
		default: return fsd_command_response_t{.return_code = INT64_MIN};
	}
}

[[noreturn]] int fsd_start(void *online_sem, long fsd_command_mailbox) {
	sem_post(online_sem);

	while (true) {
		threads::message_t recv_buf;
		recv_msg(fsd_command_mailbox, UINT64_MAX, &recv_buf);
		auto command  = *reinterpret_cast<fsd_command_t *>(&recv_buf.data);
		auto response = process_command(command);
		memcpy(recv_buf.data, &response, sizeof(response));
		send_reply(&recv_buf);
	}
}