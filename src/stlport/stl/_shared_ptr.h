#ifndef _STLP_INTERNAL_SHARED_PTR_H
#define _STLP_INTERNAL_SHARED_PTR_H

#include <stl/config/features.h>
#include <stdatomic.h>
#include <new>

_STLP_BEGIN_NAMESPACE

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
			this->ptr = nullptr;
		}

		weak_ptr(const weak_ptr& r) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->weak_count, 1);
			}
		}

		template<class Y> weak_ptr(const shared_ptr<Y>& r) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->weak_count, 1);
			}
		}

		weak_ptr(weak_ptr&& r ) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = nullptr;
			r.ptr = nullptr;
		}

		template<class Y> weak_ptr(weak_ptr<Y>&& r ) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = nullptr;
			r.ptr = nullptr;
		}

		~weak_ptr() {
			this->reset();
		}

		weak_ptr& operator=(weak_ptr&& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = nullptr;
			r.ptr = nullptr;
			return *this;
		}

		weak_ptr& operator=(weak_ptr& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->weak_count, 1);
			}
			return *this;
		}

		void reset() noexcept {
			if (this->state != nullptr) {
				if (atomic_fetch_sub(&this->state->weak_count, 1) == 1) {
					if (atomic_load(&this->state->strong_count) == 0) {
						delete this->state;
					}
				}
			}
			this->state = nullptr;
			this->ptr = nullptr;
		}

		void swap(weak_ptr& r) noexcept {
			shared_ptr_state *tmp_state = this->state;
			T *tmp_ptr = this->ptr;
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = tmp_state;
			r.ptr = tmp_ptr;
		}

		long use_count() const noexcept { return atomic_load(&this->state->strong_count); }
		bool expired() const noexcept { return this->state->expired; }

		shared_ptr<T> lock() const noexcept {
			return this->expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
		}
};

template<class T> class shared_ptr {
	public:
		shared_ptr_state *state;
		T *ptr;
	public:
		using weak_type = weak_ptr<T>;
		using element_type = T;

		constexpr shared_ptr() noexcept {
			this->state = nullptr;
			this->ptr = nullptr;
		}

		shared_ptr(const shared_ptr& r) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->strong_count, 1);
			}
		}

		template<class Y> shared_ptr(const shared_ptr<Y>& r) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->strong_count, 1);
			}
		}

		shared_ptr(shared_ptr&& r ) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = nullptr;
			r.ptr = nullptr;
		}

		template<class Y> shared_ptr(shared_ptr<Y>&& r ) noexcept {
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = nullptr;
			r.ptr = nullptr;
		}

		template<class Y> explicit shared_ptr(Y* ptr) {
			this->ptr = ptr;
			this->state = new shared_ptr_state();
		}

		template<class Y> explicit shared_ptr(const weak_ptr<Y>& r) {
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->strong_count, 1);
			}
		}

		~shared_ptr() {
			this->reset();
		}

		shared_ptr& operator=(shared_ptr&& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = nullptr;
			r.ptr = nullptr;
			return *this;
		}

		shared_ptr& operator=(shared_ptr& r) noexcept {
			this->reset();
			this->state = r.state;
			this->ptr = r.ptr;
			if (this->state != nullptr) {
				atomic_fetch_add(&this->state->strong_count, 1);
			}
			return *this;
		}

		void swap(shared_ptr& r) noexcept {
			shared_ptr_state *tmp_state = this->state;
			T *tmp_ptr = this->ptr;
			this->state = r.state;
			this->ptr = r.ptr;
			r.state = tmp_state;
			r.ptr = tmp_ptr;
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
			this->ptr = nullptr;
			this->state = nullptr;
		}
		
		template<class Y> void reset(Y* ptr) {
			this->reset();
			this->ptr = ptr;
			this->state = new shared_ptr_state();
		}

		element_type* get() const noexcept { return this->ptr; }
		T& operator*() const noexcept { return *this->ptr; }
		T* operator->() const noexcept { return this->ptr; }
		
		long use_count() const noexcept { return atomic_load(&this->state->strong_count); }
		explicit operator bool() const noexcept { return this->ptr != nullptr; }
};

template<class T, class... Args> shared_ptr<T> make_shared(Args&&... args) {
	return shared_ptr<T>(new T(args...));
}

template<class T, class U> bool operator==(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept { return lhs.get() == rhs.get(); }
template<class T, class U> bool operator!=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept { return lhs.get() != rhs.get(); }
template<class T, class U> bool operator>=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept { return lhs.get() >= rhs.get(); }
template<class T, class U> bool operator>(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept { return lhs.get() > rhs.get(); }
template<class T, class U> bool operator<=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept { return lhs.get() <= rhs.get(); }
template<class T, class U> bool operator<(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept { return lhs.get() < rhs.get(); }
template<class T> void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) noexcept { lhs.swap(rhs); }
template<class T> void swap(weak_ptr<T>& lhs, weak_ptr<T>& rhs) noexcept { lhs.swap(rhs); }

//template <class T, class U> std::strong_ordering operator<=>(const std::shared_ptr<T>& lhs, const std::shared_ptr<U>& rhs) noexcept {}

_STLP_END_NAMESPACE

#endif
