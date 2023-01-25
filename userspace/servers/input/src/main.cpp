/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "keyboard_server.hpp"

namespace {
	using namespace ipc::popcorn::input::keyboard;

	class InputServer : public ipc::popcorn::input::keyboard::Server {
	protected:
		key_down_return key_down(Scancode) override {}
		key_up_return key_up(Scancode) override {}
	};
}   // namespace


int main() {
	InputServer server{};
	server.main_loop();
}
