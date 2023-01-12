
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STL__STL_UNORDERED_MAP_HPP
#define POPCORN_KERNEL_SRC_STL__STL_UNORDERED_MAP_HPP

#include <functional>

HUGOS_STL_BEGIN_NAMESPACE
	template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>> class unordered_map {
	private:
		struct node_t {
			node_t *next;
			bool valid;
			Key key;
			T data;
		};

	public:
		using size_type  = std::size_t;
		using key_equal  = KeyEqual;
		using value_type = std::pair<const Key, T>;

		unordered_map() : unordered_map(3) {}
		explicit unordered_map(size_type bucket_count, const Hash& hash = Hash(), const key_equal& equal = key_equal())
			: start(reinterpret_cast<node_t *>(::operator new(sizeof(node_t) * bucket_count))),
			  end(this->start + bucket_count),
			  hasher(hash),
			  equaler(equal) {}

		size_type bucket_count() { return this->end - this->start; }

		void insert(const value_type& value);
		T& operator[](Key key);

	private:
		node_t *start;
		node_t *end;
		[[no_unique_address]] Hash hasher;
		[[no_unique_address]] KeyEqual equaler;

		void move_items(node_t *new_buf)
				requires(std::is_copy_constructible_v<Key> && std::is_copy_constructible_v<T>
		                 && (!std::is_nothrow_move_constructible_v<T> || !std::is_nothrow_move_constructible_v<Key>))
		{
			for (size_type i = 0; i < this->bucket_count(); i++) {
				if (this->start[i].valid) {
					new (&new_buf[i].next) node_t *(this->start[i].next);
					new (&new_buf[i].valid) bool(true);
					new (&new_buf[i].key) Key(this->start[i].key);
					new (&new_buf[i].data) T(this->start[i].data);
					this->start[i].key.~Key();
					this->start[i].key.~T();
				}
			}
		}

		void move_items(node_t *new_buf) noexcept
				requires(std::is_nothrow_move_constructible_v<Key> && std::is_nothrow_move_constructible_v<T>)
		{
			for (size_type i = 0; i < this->bucket_count(); i++) {
				if (this->start[i].valid) {
					new (&new_buf[i].next) node_t *(this->start[i].next);
					new (&new_buf[i].valid) bool(true);
					new (&new_buf[i].key) Key(std::move(this->start[i].key));
					new (&new_buf[i].data) T(std::move(this->start[i].data));
					this->start[i].key.~Key();
					this->start[i].key.~T();
				}
			}
		}

		void expand_to(size_type new_capacity) {
			node_t *new_buf_start = reinterpret_cast<node_t *>(::operator new(sizeof(node_t) * new_capacity));
			move_items(new_buf_start);
			node_t *old_buf_start = this->buffer_start_;
			this->buffer_start_   = new_buf_start;
			this->buffer_end_     = new_buf_start + new_capacity;
			operator delete(old_buf_start);
		}

		void expand() {
			size_type new_capacity = ;   // this->capacity() * 3 / 2 + 1;
			this->expand_to(new_capacity);
		}
	};
HUGOS_STL_END_NAMESPACE

#endif   // POPCORN_KERNEL_SRC_STL__STL_UNORDERED_MAP_HPP
