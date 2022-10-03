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

void test(void *sem) {
	journal_log("test started!\n");
	for (uint32_t i = 0; i < 1 << 28; i++);
	journal_log("posting to sem!\n");
	sem_post(sem);
	while (1);
}

int fsd_main(void *online_sem) {
	journal_log("fsd started!\n");
	void *test_sem = sem_init(1);
	sys_spawn("test", test, test_sem);
	journal_log("fsd waiting for sem\n");
	//sem_wait(test_sem);
	journal_log("fsd back\n");
	sem_post(online_sem);
	while (1);
}