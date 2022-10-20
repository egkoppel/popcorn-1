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

	union {
		struct {

		} read;

		struct {

		} write;

		struct {

		} open;

		struct {

		} close;

		struct {
			uint32_t driver_command_len;
			char mountpoint;
			char driver_info[247];
		} mount;

		struct {

		} umount;

		struct {
			uint32_t idx;
			char str[248];
		} string_extra;
	} data;
};
static_assert(sizeof(fsd_command_t) == 256);

extern "C" struct fsd_command_response_t {
	int64_t return_code;
	[[maybe_unused]] char reserved[248];
};
static_assert(sizeof(fsd_command_response_t) == 256);

#endif //HUG_FSD_HPP
