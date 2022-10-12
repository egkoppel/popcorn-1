
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mailing.hpp"

alignas(alignof(std::map<uint64_t, threads::Mailbox>)) static char mailboxes_[sizeof(std::map<uint64_t, threads::Mailbox>)]; // memory for the stream object
auto& threads::mailboxes = reinterpret_cast<std::map<uint64_t, Mailbox>&>(mailboxes_);

threads::Mailbox *threads::get_mailbox(uint64_t pid) {
	if (auto task = mailboxes.find(pid); task != mailboxes.end()) {
		return &task->second;
	} else {
		return nullptr;
	}
}

void threads::new_mailbox(uint64_t pid) {
	mailboxes.insert({pid, Mailbox()});
}
