/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_FSD_HPP
#define HUG_FSD_HPP

#include <stdint.h>

[[noreturn]] int fsd_start(void *online_sem, long fsd_command_mailbox);

extern "C" struct fsd_command_t {
	enum : uint32_t {
		READ,
		WRITE,
		OPEN,
		CLOSE,
		MOUNT,
		UMOUNT,
		STRING_EXTRA
	} command;

	char data[252];
};
struct fsd_command_read_t {
	uint64_t fd;
	uint64_t size;
};
struct fsd_command_write_t {

};
struct fsd_command_open_t {
	uint32_t path_len;
	char path[248];
};
struct fsd_command_close_t {

};
struct fsd_command_mount_t {
	uint32_t driver_command_len;
	char mountpoint;
	char driver_info[247];
};
struct fsd_command_umount_t {

};
struct fsd_command_string_extra_t {
	uint32_t idx;
	char str[248];
};
static_assert(sizeof(fsd_command_t) == 256);
static_assert(sizeof(fsd_command_read_t) <= 252);
static_assert(sizeof(fsd_command_write_t) <= 252);
static_assert(sizeof(fsd_command_open_t) <= 252);
static_assert(sizeof(fsd_command_close_t) <= 252);
static_assert(sizeof(fsd_command_mount_t) <= 252);
static_assert(sizeof(fsd_command_umount_t) <= 252);
static_assert(sizeof(fsd_command_string_extra_t) <= 252);

extern "C" struct fsd_command_response_t {
	int64_t return_code;

	union {
		struct {
			uint64_t fd;
		} open;
		struct {
			int64_t mmap_vm_handle;
		} read;
	} data;

	[[maybe_unused]] char reserved[240];
};
static_assert(sizeof(fsd_command_response_t) == 256);

#endif //HUG_FSD_HPP
