
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_VECTOR_HPP
#define HUGOS_KERNEL_SRC_STL__STL_VECTOR_HPP

#include "pointer_iterator_wrapper.hpp"

#include <iterator>

HUGOS_STL_BEGIN_NAMESPACE
	template<class T> class vector {
		template<class U> friend class vector;

	public:
		using value_type             = T;
		using size_type              = size_t;
		using difference_type        = ptrdiff_t;
		using reference              = value_type&;
		using const_reference        = const value_type&;
		using iterator               = detail::iterator_ptr_wrapper<value_type>;
		using const_iterator         = detail::iterator_ptr_wrapper<const value_type>;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	private:
		T *buffer_start_;
		T *buffer_end_;
		size_t item_count_;

		void move_items(T *new_buf) requires(std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>)
		{
			for (size_type i = 0; i < this->size(); i++) {
				new (&new_buf[i]) T(this->buffer_start_[i]);
				this->buffer_start_[i].~T();
			}
		}

		void move_items(T *new_buf) noexcept requires(std::is_nothrow_move_constructible_v<T>)
		{
			for (size_type i = 0; i < this->size(); i++) {
				new (&new_buf[i]) T(std::move(this->buffer_start_[i]));
				this->buffer_start_[i].~T();
			}
		}

		void expand_to(size_type new_capacity) {
			T *new_buf_start = reinterpret_cast<T *>(operator new(sizeof(T) * new_capacity));
			move_items(new_buf_start);
			T *old_buf_start    = this->buffer_start_;
			this->buffer_start_ = new_buf_start;
			this->buffer_end_   = new_buf_start + new_capacity;
			operator delete(old_buf_start);
		}

		void expand() {
			size_type new_capacity = this->capacity() * 3 / 2 + 1;
			this->expand_to(new_capacity);
		}

	public:
		constexpr vector() noexcept : buffer_start_(nullptr), buffer_end_(nullptr), item_count_(0) {}
		constexpr vector(size_type count, const T& value)
			: buffer_start_(nullptr),
			  buffer_end_(nullptr),
			  item_count_(0) {
			for (size_type i = 0; i < count; i++) this->push_back(value);
		}
		constexpr explicit vector(size_type count)
			: buffer_start_(reinterpret_cast<T *>(operator new(count * sizeof(T)))),
			  buffer_end_(buffer_start_ + count),
			  item_count_(0) {}
		constexpr vector(vector& other);
		constexpr vector(vector&& other) noexcept {
			this->buffer_start_ = other.buffer_start_;
			this->buffer_end_   = other.buffer_end_;
			this->item_count_   = other.item_count_;
			other.buffer_start_ = nullptr;
			other.buffer_end_   = nullptr;
			other.item_count_   = 0;
		}
		vector(std::initializer_list<T> init) : vector(init.begin(), init.end()) {}

		template<class InputIt>
		vector(InputIt begin, InputIt end) requires(requires() { *begin++; })
			: buffer_start_(nullptr),
			  buffer_end_(nullptr),
			  item_count_(0) {
			for (; begin < end; begin++) this->push_back(*begin);
		}

		~vector() {
			for (size_type i = 0; i < this->size(); i++) { this->buffer_start_[i].~T(); }
			operator delete(this->buffer_start_);
		}

		template<class... Args> constexpr reference emplace_back(Args&&...args) {
			if (this->size() == this->capacity()) this->expand();
			new (&this->buffer_start_[this->item_count_]) T(std::forward<Args>(args)...);
			this->item_count_++;
			return this->back();
		}

		constexpr void push_back(const T& value) {
			if (this->size() == this->capacity()) this->expand();
			new (&this->buffer_start_[this->item_count_]) T(value);
			this->item_count_++;
		}

		constexpr void push_back(T&& value) {
			if (this->size() == this->capacity()) this->expand();
			new (&this->buffer_start_[this->item_count_]) T(std::forward<T>(value));
			this->item_count_++;
		}

		constexpr reference operator[](size_type pos) { return this->buffer_start_[pos]; }

		constexpr iterator begin() noexcept { return iterator(this->buffer_start_); }
		constexpr iterator end() noexcept { return iterator(this->buffer_start_ + this->size()); }
		constexpr const_iterator begin() const noexcept { return this->cbegin(); }
		constexpr const_iterator end() const noexcept { return this->cend(); }
		constexpr const_iterator cbegin() const noexcept { return const_iterator(this->buffer_start_); }
		constexpr const_iterator cend() const noexcept { return const_iterator(this->buffer_start_ + this->size()); }

		constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(this->buffer_start_ + this->size() - 1);
		}
		constexpr reverse_iterator rend() noexcept { return reverse_iterator(this->buffer_start_ - 1); }
		constexpr const_reverse_iterator rbegin() const noexcept { return this->crbegin(); }
		constexpr const_reverse_iterator rend() const noexcept { return this->crend(); }
		constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(this->buffer_start_ + this->size() - 1);
		}
		constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(this->buffer_start_ - 1);
		}

		constexpr reference front() { return *this->buffer_start_; }
		constexpr reference back() { return *(this->buffer_start_ + this->size() - 1); }

		constexpr size_type capacity() const noexcept { return this->buffer_end_ - this->buffer_start_; }
		constexpr size_type size() const noexcept { return this->item_count_; }
		constexpr bool empty() const noexcept { return this->size() == 0; }

		constexpr void reserve(size_type new_capacity) { this->expand_to(new_capacity); }
	};
HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_VECTOR_HPP
