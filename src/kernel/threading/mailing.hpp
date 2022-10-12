
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

#define MAX_MAILBOX_SIZE 16
#define MESSAGE_SIZE 256

namespace threads {
	using message_t = struct { char _[MESSAGE_SIZE]; };

	class Mailbox {
	private:
		std::deque<message_t> mail;
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
	};

	Mailbox *get_mailbox(uint64_t pid);
	void new_mailbox(uint64_t pid);

	extern std::map<uint64_t, Mailbox>& mailboxes;
}

#endif //HUG_MAILING_HPP
