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

[[noreturn]] int fsd_start(void *online_sem) {
	journal_log("fsd started!\n");
	journal_log("fsd online\nmounting ramdisk\n");

	__asm__ volatile("int3");
	while (1) __asm__ volatile("");

	sem_post(online_sem);
	threads::message_t recv;
	wait_msg(&recv);

	while (1);// __asm__ volatile("");
	return -1;
}