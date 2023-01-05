
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_DEQUE_HPP
#define HUGOS_KERNEL_SRC_STL__STL_DEQUE_HPP

HUGOS_STL_BEGIN_NAMESPACE
	template<class T> class deque {
	public:
		using value_type      = T;
		using size_type       = size_t;
		using difference_type = ptrdiff_t;
		using reference       = value_type&;
		using const_reference = const value_type&;

	private:
		struct node {
			node *next;
			T data;

			template<class U = T> node(U&& data) : data(std::forward<U>(data)), next(nullptr) {}
		};

		node *head;
		node *tail;
		size_type size_;

	public:
		constexpr deque() noexcept : head(nullptr), tail(nullptr), size_(0) {}
		constexpr deque(size_type count, const T& value) requires(std::is_copy_constructible_v<T>)
			: deque() {
			for (size_type i = 0; i < count; ++i) { this->push_front(value); }
		}
		constexpr explicit deque(size_type count);
		constexpr deque(deque&& other) noexcept : head(other.head), tail(other.tail) {
			other.head = nullptr;
			other.tail = nullptr;
		}
		constexpr deque(std::initializer_list<T> init) : deque() {
			for (auto&& item : init) { this->push_back(std::forward<decltype(item)>(item)); }
		}
		constexpr ~deque() {
			auto current = this->head;
			while (current) {
				auto old = current;
				current  = current->next;
				delete old;
			}
		}

		reference front() { return this->head->data; }
		const_reference front() const { return this->head->data; }
		reference back() { return this->tail->data; }
		const_reference back() const { return this->tail->data; }

		[[nodiscard]] bool empty() const noexcept { return this->size() == 0; }
		size_type size() const noexcept { return this->size_; }

		template<class... Args> reference emplace_front(Args&&...args);
		template<class... Args> reference emplace_back(Args&&...args);

		void push_front(const T& value) {
			auto new_head = new node{value};
			this->push_front_(new_head);
		}

		void push_back(const T& value) {
			auto new_tail = new node{value};
			this->push_back_(new_tail);
		}

		void push_front(T&& value) {
			auto new_head = new node{std::move(value)};
			this->push_front_(new_head);
		}

		void push_back(T&& value) {
			auto new_tail = new node{std::move(value)};
			this->push_back_(new_tail);
		}

		void pop_front() {
			auto old_head = this->head;
			this->head    = old_head->next;
			if (this->tail == old_head) this->tail = nullptr;
			delete old_head;
			this->size_--;
		}

		void pop_back();

	private:
		void push_front_(node *new_head) noexcept {
			new_head->next = this->head;

			if (this->head == nullptr) {
				assert(this->tail == nullptr);
				this->tail = new_head;
			}
			this->head = new_head;
			this->size_++;
		}
		void push_back_(node *new_tail) noexcept {
			if (this->empty()) {
				assert(this->tail == nullptr);
				this->head = new_tail;
			} else this->tail->next = new_tail;

			this->tail = new_tail;
			this->size_++;
		}
	};
HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_DEQUE_HPP
