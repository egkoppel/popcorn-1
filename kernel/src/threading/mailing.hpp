
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MAILING_HPP
#define HUGOS_MAILING_HPP

#include <stdint.h>

#define MAX_MAILBOX_SIZE 16
#define MESSAGE_SIZE     256
namespace threads {
	using message_t = struct {
		int64_t sender;
		char data[MESSAGE_SIZE];
	};
}   // namespace threads

#include "task.hpp"

#include <map>
#include <memory>
#include <utility/handle_table.hpp>
#include <utility/ring_buffer.hpp>
#include <utility>

namespace threads {
	class Mailbox {
	private:
		structures::ring_buffer<message_t, MAX_MAILBOX_SIZE> mail;
		Task *task;

	public:
		inline int send_message(message_t *message) { return -!mail.push(*message); }

		inline int get_message(message_t *message_buf) {
			if (mail.empty()) return -1;
			*message_buf = mail.pop();
			return 0;
		}

		explicit Mailbox(Task& for_task) : task(&for_task) {}
		inline Task& get_owning_task() const { return *this->task; };
		inline void set_owning_task(Task *task) { this->task = task; };
	};

	Mailbox *get_mailbox(syscall_handle_t handle);
	syscall_handle_t new_mailbox(Task& for_task);
	void destroy_mailbox(syscall_handle_t handle);

	void mailboxes_init();
}   // namespace threads

#endif   //HUGOS_MAILING_HPP
