
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_MAILING_HPP
#define HUG_MAILING_HPP

#include <deque>
#include <map>
#include <memory>
#include <utility>
#include "../interrupts/handle_table.hpp"
#include "threading.hpp"

#define MAX_MAILBOX_SIZE 16
#define MESSAGE_SIZE 256

namespace threads {
	using message_t = struct { uint64_t sender; char data[MESSAGE_SIZE]; };

	class Mailbox {
	private:
		std::deque<message_t> mail;
		std::shared_ptr<Task> task;
	public:
		inline int send_message(message_t *message) {
			if (mail.size() >= MAX_MAILBOX_SIZE) return -1;
			mail.push_back(*message);
			return 0;
		}

		inline int get_message(message_t *message_buf) {
			if (mail.empty()) return -1;
			*message_buf = mail.front();
			mail.pop_front();
			return 0;
		}

		explicit Mailbox(std::shared_ptr<Task> for_task) : task(for_task) {}
		inline std::shared_ptr<Task> get_owning_task() const { return this->task; };
		inline void set_owning_task(std::shared_ptr<Task> task) { this->task = task; };
	};

	Mailbox *get_mailbox(syscall_handle_t handle);
	syscall_handle_t new_mailbox(std::shared_ptr<Task> for_task);
	void destroy_mailbox(syscall_handle_t handle);

	void mailboxes_init();
}

#endif //HUG_MAILING_HPP
