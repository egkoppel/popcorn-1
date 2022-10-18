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

[[noreturn]] int fsd_start(void *online_sem, void *ramfs_data, uint64_t ramfs_size) {
	Initramfs ramfs((uint64_t)ramfs_data, (uint64_t)ramfs_data + ramfs_size);

	void *data;
	size_t data_len = ramfs.locate_file("initramfs/.placeholder", &data);

	sem_post(online_sem);

	while (true) __asm__ volatile("");
}