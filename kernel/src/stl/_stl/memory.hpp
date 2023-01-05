
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_MEMORY_HPP
#define HUGOS_KERNEL_SRC_STL__STL_MEMORY_HPP

#include <compare>
#include <stdatomic.h>
#include <type_traits>
#include <utility>

HUGOS_STL_BEGIN_NAMESPACE

	template<class T> struct allocator {
	public:
		using value_type      = T;
		using size_type       = size_t;
		using difference_type = ptrdiff_t;

		constexpr allocator() noexcept = default;

		[[nodiscard]] constexpr T *allocate(size_t n) { return ::operator new(n * sizeof(T)); }
		constexpr void deallocate(T *p, size_t) { ::operator delete(p); }
	};

	template<class T> struct default_delete {
	public:
		constexpr default_delete() noexcept = default;
		// template<class U> constexpr default_delete(const default_delete<U>& d) noexcept {}
		constexpr void operator()(T *ptr) const { delete ptr; }
	};

	template<class T> class [[clang::trivial_abi]] unique_ptr {
		template<class U> friend class unique_ptr;

	private:
		T *ptr;

	public:
		using pointer      = T *;
		using element_type = T;

		constexpr unique_ptr() noexcept { this->ptr = nullptr; }
		explicit unique_ptr(pointer p) noexcept { this->ptr = p; }
		unique_ptr(unique_ptr<T>&& u) noexcept {
			this->ptr = u.ptr;
			u.ptr     = nullptr;
		}
		template<class U> unique_ptr(unique_ptr<U>&& u) noexcept {
			this->ptr = u.ptr;
			u.ptr     = nullptr;
		}
		unique_ptr(unique_ptr& u) = delete;
		unique_ptr(decltype(nullptr)) noexcept { this->ptr = nullptr; }
		~unique_ptr() {
			delete this->ptr;
			this->ptr = nullptr;
		}

		pointer release() noexcept {
			pointer p = this->ptr;
			this->ptr = nullptr;
			return p;
		}

		void reset(pointer ptr_ = pointer()) noexcept {
			delete this->ptr;
			this->ptr = ptr_;
		}

		void swap(unique_ptr& other) noexcept {
			pointer tmp = this->ptr;
			this->ptr   = other.ptr;
			other.ptr   = tmp;
		}

		unique_ptr& operator=(unique_ptr<T>&& u) noexcept {
			delete this->ptr;
			this->ptr = u.ptr;
			u.ptr     = nullptr;
			return *this;
		}

		unique_ptr& operator=(decltype(nullptr)) noexcept {
			delete this->ptr;
			this->ptr = nullptr;
		}

		pointer get() const noexcept { return this->ptr; }
		explicit operator bool() const noexcept { return this->ptr != nullptr; }
		pointer operator->() const noexcept { return this->ptr; }
		typename std::add_lvalue_reference<T>::type operator*() const noexcept { return *this->ptr; }

		template<class U, class... Args> friend unique_ptr<U> make_unique(Args&&...args);
	};

	template<class U, class... Args> unique_ptr<U> make_unique(Args && ...args) {
		return unique_ptr<U>(new U(std::forward<Args>(args)...));
	}

	template<class T, class U>
	std::strong_ordering operator<=>(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept {
		return lhs.get() <=> rhs.get();
	}
	template<class T, class U> bool operator!=(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept {
		return lhs.get() != rhs.get();
	}
	template<class T> bool operator==(const unique_ptr<T>& lhs, decltype(nullptr)) noexcept {
		return !lhs.get();
	}

	template<class T> void swap(unique_ptr<T> & lhs, unique_ptr<T> & rhs) noexcept {
		lhs.swap(rhs);
	}

	class shared_ptr_state {
	public:
		bool expired;
		atomic_ulong strong_count;
		atomic_ulong weak_count;

		shared_ptr_state() {
			expired = false;
			atomic_init(&this->strong_count, 1);
			atomic_init(&this->weak_count, 0);
		}
	};

	template<class T> class shared_ptr;

	template<class T> class weak_ptr {
	public:
		shared_ptr_state *state;
		T *ptr;

	public:
		using element_type = T;

		constexpr weak_ptr() noexcept {
			this->state = nullptr;
			this->ptr   = nullptr;
		}

		weak_ptr(const weak_ptr& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->weak_count, 1); }
		}

		template<class Y> weak_ptr(const shared_ptr<Y>& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->weak_count, 1); }
		}

		weak_ptr(weak_ptr&& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			r.state     = nullptr;
			r.ptr       = nullptr;
		}

		template<class Y> weak_ptr(weak_ptr<Y>&& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			r.state     = nullptr;
			r.ptr       = nullptr;
		}

		~weak_ptr() { this->reset(); }

		weak_ptr& operator=(weak_ptr&& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr   = r.ptr;
			r.state     = nullptr;
			r.ptr       = nullptr;
			return *this;
		}

		weak_ptr& operator=(weak_ptr& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->weak_count, 1); }
			return *this;
		}

		void reset() noexcept {
			if (this->state != nullptr) {
				if (atomic_fetch_sub(&this->state->weak_count, 1) == 1) {
					if (atomic_load(&this->state->strong_count) == 0) { delete this->state; }
				}
			}
			this->state = nullptr;
			this->ptr   = nullptr;
		}

		void swap(weak_ptr& r) noexcept {
			shared_ptr_state *tmp_state = this->state;
			T *tmp_ptr                  = this->ptr;
			this->state                 = r.state;
			this->ptr                   = r.ptr;
			r.state                     = tmp_state;
			r.ptr                       = tmp_ptr;
		}

		long use_count() const noexcept { return atomic_load(&this->state->strong_count); }
		bool expired() const noexcept { return this->state->expired; }

		shared_ptr<T> lock() const noexcept { return this->expired() ? shared_ptr<T>() : shared_ptr<T>(*this); }
	};

	template<class T> class shared_ptr {
	public:
		shared_ptr_state *state;
		T *ptr;

	public:
		using weak_type    = weak_ptr<T>;
		using element_type = T;

		constexpr shared_ptr() noexcept {
			this->state = nullptr;
			this->ptr   = nullptr;
		}

		constexpr shared_ptr(decltype(nullptr)) noexcept {
			this->state = nullptr;
			this->ptr   = nullptr;
		}

		shared_ptr(const shared_ptr& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->strong_count, 1); }
		}

		template<class Y> shared_ptr(const shared_ptr<Y>& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->strong_count, 1); }
		}

		shared_ptr(shared_ptr&& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			r.state     = nullptr;
			r.ptr       = nullptr;
		}

		template<class Y> shared_ptr(shared_ptr<Y>&& r) noexcept {
			this->state = r.state;
			this->ptr   = r.ptr;
			r.state     = nullptr;
			r.ptr       = nullptr;
		}

		template<class Y> explicit shared_ptr(Y *ptr) {
			this->ptr   = ptr;
			this->state = new shared_ptr_state();
		}

		template<class Y> explicit shared_ptr(const weak_ptr<Y>& r) {
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->strong_count, 1); }
		}

		~shared_ptr() { this->reset(); }

		shared_ptr& operator=(shared_ptr&& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr   = r.ptr;
			r.state     = nullptr;
			r.ptr       = nullptr;
			return *this;
		}

		shared_ptr& operator=(const shared_ptr& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr   = r.ptr;
			if (this->state != nullptr) { atomic_fetch_add(&this->state->strong_count, 1); }
			return *this;
		}

		void swap(shared_ptr& r) noexcept {
			shared_ptr_state *tmp_state = this->state;
			T *tmp_ptr                  = this->ptr;
			this->state                 = r.state;
			this->ptr                   = r.ptr;
			r.state                     = tmp_state;
			r.ptr                       = tmp_ptr;
		}

		void reset() noexcept {
			if (this->state != nullptr) {
				if (atomic_fetch_sub(&this->state->strong_count, 1) == 1) {
					delete this->ptr;
					if (atomic_load(&this->state->weak_count) == 0) {
						delete this->state;
					} else {
						this->state->expired = true;
					}
				}
			}
			this->ptr   = nullptr;
			this->state = nullptr;
		}

		template<class Y> void reset(Y *ptr_) {
			this->reset();
			this->ptr   = ptr_;
			this->state = new shared_ptr_state();
		}

		element_type *get() const noexcept { return this->ptr; }
		T& operator*() const noexcept { return *this->ptr; }
		T *operator->() const noexcept { return this->ptr; }

		[[nodiscard]] long use_count() const noexcept { return atomic_load(&this->state->strong_count); }
		explicit operator bool() const noexcept { return this->ptr != nullptr; }
	};

	template<class T, class... Args> shared_ptr<T> make_shared(Args && ...args) {
		return shared_ptr<T>(new T(args...));
	}

	template<class T, class U>
	std::strong_ordering operator<=>(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept {
		return lhs.get() <=> rhs.get();
	}
	template<class T, class U> bool operator!=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept {
		return lhs.get() != rhs.get();
	}


	template<class T> bool operator==(const shared_ptr<T>& lhs, decltype(nullptr)) noexcept {
		return !lhs.get();
	}

	template<class T> void swap(shared_ptr<T> & lhs, shared_ptr<T> & rhs) noexcept {
		lhs.swap(rhs);
	}
	template<class T> void swap(weak_ptr<T> & lhs, weak_ptr<T> & rhs) noexcept {
		lhs.swap(rhs);
	}
HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_MEMORY_HPP
