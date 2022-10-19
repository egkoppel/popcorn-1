
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_HANDLE_TABLE_HPP
#define HUG_HANDLE_TABLE_HPP

#include <map>
#include <deque>

extern "C" struct syscall_handle_t {
	int64_t data;

private:
	union int_uint64_t { uint64_t u; int64_t i; };
public:
	syscall_handle_t(int64_t i) : data(i) {};

	syscall_handle_t(uint64_t sign, uint64_t type, uint64_t epoch, uint64_t index) {
		uint64_t d = (sign & 1) << 63 | (type & 0x7FFF) << 48 | (epoch & 0xFFFF) << 32 | (index & 0xFFFFFFFF);
		int_uint64_t a = {.u = d};
		this->data = a.i;
	}

	operator int64_t() const {
		return this->data;
	}

	inline uint64_t get_type() const {
		int_uint64_t a = {.i = this->data};
		return (a.u & (0x7FFFull << 48)) >> 48;
	}

	inline uint64_t get_epoch() const {
		int_uint64_t a = {.i = this->data};
		return (a.u & (0xFFFFull << 32)) >> 32;
	}

	inline uint64_t get_index() const {
		int_uint64_t a = {.i = this->data};
		return (a.u & (0xFFFFFFFFull << 0)) >> 0;
	}
};
static_assert(sizeof(syscall_handle_t) == 8);

namespace syscall_handle_type {
	enum syscall_handle_type {
		NONE = 0x0,
		TASK = 0x1,
		VM = 0x2,
		MAILBOX = 0x3
	};
}

template<class T, syscall_handle_type::syscall_handle_type handle_type> class SyscallHandleTable {
private:
	std::map<uint64_t, T> table;
	std::map<uint64_t, uint16_t> epochs;
	std::deque<uint64_t> free_keys;
	uint64_t next_key;

	uint64_t get_free_key() {
		if (!free_keys.empty()) {
			uint64_t k = free_keys.front();
			free_keys.pop_front();
			return k;
		}
		// TODO: add some timeout here
		while (table.find(this->next_key) != table.end()) {
			this->next_key++;
		}
		return this->next_key++;
	}

	void return_key(uint64_t key) {
		if (free_keys.size() < 16) {
			free_keys.push_back(key);
		}
	}
public:
	SyscallHandleTable() : next_key(0), table(), epochs(), free_keys() {}

	template<class E> T& get_data_from_handle(syscall_handle_t handle, E error_val) {
		if (handle <= 0) return error_val;
		if (handle.get_type() != handle_type) return error_val;

		auto data = this->table.find(handle.get_index());
		auto epoch = this->epochs.find(handle.get_index());

		if (data == this->table.end()) return error_val;
		if (epoch == this->epochs.end()) return error_val;
		if (handle.get_epoch() != epoch->second) return error_val;

		return data->second;
	}

	T *get_data_from_handle_ptr(syscall_handle_t handle) {
		if (handle <= 0) return nullptr;
		if (handle.get_type() != handle_type) return nullptr;

		auto data = this->table.find(handle.get_index());
		auto epoch = this->epochs.find(handle.get_index());

		if (data == this->table.end()) return nullptr;
		if (epoch == this->epochs.end()) return nullptr;
		if (handle.get_epoch() != epoch->second) return nullptr;

		return &data->second;
	}

	void free_handle(syscall_handle_t handle) {
		if (handle <= 0) return;
		if (handle.get_type() != handle_type) return;

		auto data = this->table.find(handle.get_index());
		auto epoch = this->epochs.find(handle.get_index());

		if (data == this->table.end()) return;
		if (epoch == this->epochs.end()) return;
		if (handle.get_epoch() != epoch->second) return;

		this->return_key(handle.get_epoch());
		this->table.erase(handle.get_epoch());
	}

	syscall_handle_t new_handle(T data) {
		auto index = this->get_free_key();
		auto epoch_iter = this->epochs.find(index);
		uint16_t epoch = 0;

		if (epoch_iter == this->epochs.end()) {
			this->epochs.insert({index, epoch});
		} else {
			epoch = epoch_iter->second;
		}

		this->table.insert({index, data});
		syscall_handle_t ret(0, handle_type, epoch, index);
		return ret;
	}
};

#endif //HUG_HANDLE_TABLE_HPP
