
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_RING_BUFFER_HPP
#define HUGOS_RING_BUFFER_HPP

#include <stddef.h>
#include <stdint.h>

namespace structures {
	template<class T, size_t S> class ring_buffer {
	private:
		T *buffer;
		T *data_start;
		T *data_end;
	public:
		ring_buffer() {
			this->buffer = reinterpret_cast<T *>(operator new(S * sizeof(T)));
			this->data_start = this->data_end = this->buffer;
		}
		~ring_buffer() {
			operator delete(this->buffer);
		}
		ring_buffer(ring_buffer&& other) noexcept {
			this->buffer = other.buffer;
			other.buffer = nullptr;
		}

		size_t size() { return data_end >= data_start ? data_end - data_start : S - (data_start - data_end); }
		bool empty() { return this->size() == 0; }
		bool push(T object) {
			if (this->size() == S) return false;
			*this->data_end++ = object;
			if (this->data_end >= this->buffer + S) this->data_end = this->buffer;
			return true;
		}
		T pop() {
			T ret = *this->data_start;
			this->data_start++;
			if (this->data_start >= this->buffer + S) this->data_start = this->buffer;
			return ret;
		}
	};
}

#endif //HUGOS_RING_BUFFER_HPP
