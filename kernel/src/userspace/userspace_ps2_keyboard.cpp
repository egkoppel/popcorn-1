
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "userspace_ps2_keyboard.hpp"

namespace driver::ps2_keyboard {
	int main() {
		while (true) __asm__ volatile("nop");
		return 0;
	}
}   // namespace driver::ps2_keyboard
