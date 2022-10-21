/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fsd.hpp"
#include "userspace_macros.hpp"
#include "initramfs.hpp"

struct mountpoint_t {
	int64_t driver_mailbox_handle;
};

mountpoint_t mountpoints[26];
Initramfs *initramfs;

static inline fsd_command_response_t fsd_mount(fsd_command_t *command_) {
	auto command = reinterpret_cast<fsd_command_mount_t *>(&command_->data);

	if (command->mountpoint <= 'z' && command->mountpoint >= 'a') command->mountpoint -= ('a' - 'A');
	if (command->mountpoint < 'A' || command->mountpoint > 'Z') return fsd_command_response_t{.return_code = -1};

	if (strncmp("initramfs", command->driver_info, 9) == 0) {
		mountpoints[command->mountpoint - 'A'] = {.driver_mailbox_handle = -1};
		const char *s = command->driver_info;
		while (*s++ != ' ') {};
		auto initramfs_addr_ = atoll_p(&s);
		auto initramfs_addr = *reinterpret_cast<uint64_t *>(&initramfs_addr_);
		while (*s++ != ' ') {};
		auto initramfs_size_ = atoll_p(&s);
		auto initramfs_size = *reinterpret_cast<uint64_t *>(&initramfs_size_);

		// TODO
		//initramfs = new Initramfs(initramfs_addr, initramfs_addr + initramfs_size);

		return fsd_command_response_t{.return_code = -1};
	} else {
		return fsd_command_response_t{.return_code = -2};
	}
}
static inline fsd_command_response_t fsd_open(fsd_command_t *command_) {
	auto command = reinterpret_cast<fsd_command_open_t *>(&command_->data);
	return fsd_command_response_t{.return_code = INT64_MIN};
}
static inline fsd_command_response_t fsd_close(fsd_command_t *command_) {
	auto command = reinterpret_cast<fsd_command_close_t *>(&command_->data);
	return fsd_command_response_t{.return_code = INT64_MIN};
}

static inline fsd_command_response_t process_command(fsd_command_t& command) {
	switch (command.command) {
		case fsd_command_t::MOUNT: return fsd_mount(&command);
		case fsd_command_t::OPEN: return fsd_open(&command);
		case fsd_command_t::CLOSE: return fsd_close(&command);
		default: return fsd_command_response_t{.return_code = INT64_MIN};
	}
}

[[noreturn]] int fsd_start(void *online_sem, long fsd_command_mailbox) {
	sem_post(online_sem);

	while (true) {
		threads::message_t recv_buf;
		recv_msg(fsd_command_mailbox, UINT64_MAX, &recv_buf);
		auto command = *reinterpret_cast<fsd_command_t *>(&recv_buf.data);
		auto response = process_command(command);
		memcpy(recv_buf.data, &response, sizeof(response));
		send_reply(&recv_buf);
	}
}