#ifndef _STLP_INTERNAL_UNIQUE_PTR_H
#define _STLP_INTERNAL_UNIQUE_PTR_H

#include <stl/config/features.h>
#include <stdatomic.h>
#include <new>
#include <stl/_utility.h>
#include <type_traits>

_STLP_BEGIN_NAMESPACE

template<class T> class unique_ptr {
	private:
	T *ptr;
	public:
	using pointer = T*;
	using element_type = T;

	constexpr unique_ptr() noexcept { this->ptr = nullptr; }
	explicit unique_ptr(pointer p) noexcept { this->ptr = p; }
	unique_ptr(unique_ptr&& u) noexcept {
		this->ptr = u.ptr;
		u.ptr = nullptr;
	}
	template<class U> unique_ptr(unique_ptr<U>&& u ) noexcept {
		this->ptr = u.ptr;
		u.ptr = nullptr;
	}
	unique_ptr(unique_ptr& u) = delete;
	~unique_ptr() { delete this->ptr; this->ptr = nullptr; }

	pointer release() noexcept {
		pointer p = this->ptr;
		this->ptr = nullptr;
		return p;
	}

	void reset(pointer ptr = pointer()) noexcept {
		delete this->ptr;
		this->ptr = ptr;
	}

	void swap(unique_ptr& other) noexcept {
		pointer tmp = this->ptr;
		this->ptr = other.ptr;
		other.ptr = tmp;
	}

	pointer get() const noexcept { return this->ptr; }
	explicit operator bool() const noexcept { return this->ptr != nullptr; }
	pointer operator->() const noexcept { return this->ptr; }
	typename tr1::add_lvalue_reference<T>::type operator*() const noexcept { return *this->ptr; }
};

template<class T, class... Args> unique_ptr<T> make_unique(Args&&... args) {
	return unique_ptr<T>(new T(forward<Args>(args)...));
}

template<class T, class U> bool operator==(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept { return lhs.get() == rhs.get(); }
template<class T, class U> bool operator!=(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept { return lhs.get() != rhs.get(); }
template<class T, class U> bool operator>=(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept { return lhs.get() >= rhs.get(); }
template<class T, class U> bool operator>(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept { return lhs.get() > rhs.get(); }
template<class T, class U> bool operator<=(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept { return lhs.get() <= rhs.get(); }
template<class T, class U> bool operator<(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept { return lhs.get() < rhs.get(); }
template<class T> void swap(unique_ptr<T>& lhs, unique_ptr<T>& rhs) noexcept { lhs.swap(rhs); }

_STLP_END_NAMESPACE

#endif