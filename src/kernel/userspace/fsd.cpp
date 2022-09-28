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

void test(int a) {
	journal_log("test started!\n");
	while (1);
}

int fsd_main() {
	journal_log("fsd started!\n");
	sys_spawn("test", test, 3);
	while (1);
}