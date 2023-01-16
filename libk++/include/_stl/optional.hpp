
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
// Many functions are either from or modifications of the equivalent from the LLVM project
// See LICENSE.txt in this directory for LLVM license information
// - Eliyahu Gluschove-Koppel
//
//===----------------------------------------------------------------------===//

#ifndef HUGOS_KERNEL_SRC_STL__STL_OPTIONAL_HPP
#define HUGOS_KERNEL_SRC_STL__STL_OPTIONAL_HPP

#include <concepts>
#include <exception>
#include <new>
#include <utility>

HUGOS_STL_BEGIN_NAMESPACE
	struct nullopt_t {
		explicit constexpr nullopt_t() {}
	};

	inline constexpr nullopt_t nullopt{};

	class bad_optional_access : public std::exception {
	public:
		bad_optional_access() noexcept                                 = default;
		bad_optional_access(const bad_optional_access& other) noexcept = default;
		const char *what() const noexcept override { return "std::bad_optional_access"; }
	};

	template<class T> class optional {
	private:
		bool is_some;
		alignas(alignof(T)) char inner_[sizeof(T)];   // memory for inner object
		T& inner = reinterpret_cast<T&>(inner_);

	public:
		using value_type = T;

		constexpr optional() noexcept : optional(nullopt) {}

		constexpr optional(nullopt_t) : is_some(false) {}

		constexpr optional(const optional<T>& other) requires(std::is_copy_constructible_v<T>)
			: is_some(other.is_some) {
			if (this->is_some) new (&this->inner) T(other.inner);
		}
		constexpr optional(const optional<T>& other) requires(!std::is_copy_constructible_v<T>)
		= delete;

		constexpr optional(const optional<T>&& other) noexcept requires(std::is_move_constructible_v<T>)
			: is_some(other.is_some) {
			if (this->is_some) new (&this->inner) T(std::move(*other));
		}
		constexpr optional(const optional<T>&& other) noexcept requires(!std::is_move_constructible_v<T>)
		= delete;

		template<class... Args> constexpr explicit optional(std::in_place_t, Args&&...args) : is_some(true) {
			new (&this->inner) T(args...);
		}

		template<class U = T>
		constexpr optional(U&& value) requires(!std::same_as<std::remove_reference_t<U>, optional<T>>
		                                       && !std::same_as<std::remove_reference_t<U>, std::in_place_t>)
			: is_some(true) {
			new (&this->inner) T(std::forward<U>(value));
		}

		constexpr ~optional() {
			if (this->is_some) this->inner.~T();
		}

		constexpr T *operator->() const noexcept { return &this->inner; }

		constexpr T& operator*() & noexcept { return this->inner; }

		constexpr T&& operator*() && noexcept { return this->inner; }

		constexpr T& operator*() const& noexcept { return this->inner; }

		constexpr T&& operator*() const&& noexcept { return this->inner; }

		constexpr T& value() & {
			if (*this) return this->inner;
			else throw bad_optional_access();
		}

		constexpr const T& value() const& {
			if (*this) return this->inner;
			else throw bad_optional_access();
		}

		constexpr T&& value() && {
			if (*this) return this->inner;
			else throw bad_optional_access();
		}

		constexpr const T&& value() const&& {
			if (*this) return this->inner;
			else throw bad_optional_access();
		}

		constexpr explicit operator bool() const noexcept { return this->is_some; }

		constexpr bool has_value() const noexcept { return bool(*this); }

		template<class U> constexpr T value_or(U&& default_value) const& {
			return bool(*this) ? **this : static_cast<T>(std::forward<U>(default_value));
		}
		template<class U> constexpr T value_or(U&& default_value) && {
			return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
		}

		template<class F> constexpr auto and_then(F&& f) & {
			if (*this) return f(this->inner);
			else return std::remove_cvref_t<std::invoke_result_t<F, T&>>();
		}

		template<class F> constexpr auto and_then(F&& f) const& {
			if (*this) return f(this->inner);
			else return std::remove_cvref_t<std::invoke_result_t<F, const T&>>();
		}

		template<class F> constexpr auto and_then(F&& f) && {
			if (*this) return f(std::move(this->inner));
			else return std::remove_cvref_t<std::invoke_result_t<F, T&&>>();
		}

		template<class F> constexpr auto and_then(F&& f) const&& {
			if (*this) return f(std::move(this->inner));
			else return std::remove_cvref_t<std::invoke_result_t<F, const T&&>>();
		}

		template<class F> constexpr auto transform(F&& f) & {
			using ret_t = optional<typename std::invoke_result<F, value_type&>::type>;
			if (*this) return ret_t(f(this->inner));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr auto transform(F&& f) const& {
			using ret_t = optional<typename std::invoke_result<F, const value_type&>::type>;
			if (*this) return ret_t(f(this->inner));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr auto transform(F&& f) && {
			using ret_t = optional<typename std::invoke_result<F, value_type&&>::type>;
			if (*this) return ret_t(std::move(f(std::move(this->inner))));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr auto transform(F&& f) const&& {
			using ret_t = optional<typename std::invoke_result<F, const value_type&&>::type>;
			if (*this) return ret_t(std::move(f(std::move(this->inner))));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr optional or_else(F&& f) const& { return *this ? *this : std::forward<F>(f)(); }

		template<class F> constexpr optional or_else(F&& f) && {
			return *this ? std::move(*this) : std::forward<F>(f)();
		}

		template<class U = T> constexpr optional& operator=(const optional<U>& other) {
			if (this->is_some) this->inner.~T();
			this->is_some = other.is_some;
			if (this->is_some) new (&this->inner) T(other.value());
			return *this;
		}

		template<class U = T> constexpr optional& operator=(optional<U>&& other) {
			if (this->is_some) this->inner.~T();
			this->is_some = other.is_some;

			if (other.is_some) {
				new (&this->inner) T(std::move(other.value()));
				other.inner.~T();
				other.is_some = false;
			}

			return *this;
		}

		constexpr optional& operator=(nullopt_t) {
			if (this->is_some) this->inner.~T();
			this->is_some = false;
			return *this;
		}

		template<class U = T>
		constexpr optional& operator=(U&& value)
				requires(!std::is_same_v<std::remove_cv_t<U>, optional<T>> && std::is_constructible_v<T, U>)
		{
			if (this->is_some) this->inner = std::forward<U>(value);
			else new (&this->inner) T(std::forward<U>(value));
			this->is_some = true;
			return *this;
		}
	};

	template<class T> class [[clang::trivial_abi]] optional<T *> {
	private:
		T *inner;

	public:
		using value_type = T *;

		constexpr optional() noexcept : optional(nullopt) {}
		constexpr optional(nullopt_t) : inner(nullptr) {}
		constexpr optional(const optional<T *>&)        = default;
		constexpr optional(const optional<T *>&& other) = default;

		constexpr optional(T *value) : inner(value) {}

		constexpr T *operator->() const noexcept { return this->inner; }

		constexpr T *operator*() const noexcept { return this->inner; }

		constexpr explicit operator bool() const noexcept { return this->inner; }

		constexpr T *value() const {
			if (*this) return this->inner;
			else throw bad_optional_access();
		}

		constexpr bool has_value() const noexcept { return bool(*this); }

		template<class U> constexpr T *value_or(U&& default_value) const& {
			return bool(*this) ? **this : static_cast<value_type>(std::forward<U>(default_value));
		}
		template<class U> constexpr T *value_or(U&& default_value) && {
			return bool(*this) ? std::move(**this) : static_cast<value_type>(std::forward<U>(default_value));
		}

		template<class F> constexpr auto and_then(F&& f) & {
			if (*this) return f(this->inner);
			else return std::remove_cvref_t<std::invoke_result_t<F, value_type&>>();
		}

		template<class F> constexpr auto and_then(F&& f) const& {
			if (*this) return f(this->inner);
			else return std::remove_cvref_t<std::invoke_result_t<F, const value_type&>>();
		}

		template<class F> constexpr auto and_then(F&& f) && {
			if (*this) return f(std::move(this->inner));
			else return std::remove_cvref_t<std::invoke_result_t<F, value_type&&>>();
		}

		template<class F> constexpr auto and_then(F&& f) const&& {
			if (*this) return f(std::move(this->inner));
			else return std::remove_cvref_t<std::invoke_result_t<F, const value_type&&>>();
		}

		template<class F> constexpr auto transform(F&& f) & {
			using ret_t = optional<typename std::invoke_result<F, value_type&>::type>;
			if (*this) return ret_t(f(this->inner));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr auto transform(F&& f) const& {
			using ret_t = optional<typename std::invoke_result<F, const value_type&>::type>;
			if (*this) return ret_t(f(this->inner));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr auto transform(F&& f) && {
			using ret_t = optional<typename std::invoke_result<F, value_type&&>::type>;
			if (*this) return ret_t(std::move(f(std::move(this->inner))));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr auto transform(F&& f) const&& {
			using ret_t = optional<typename std::invoke_result<F, const value_type&&>::type>;
			if (*this) return ret_t(std::move(f(std::move(this->inner))));
			else return ret_t(std::nullopt);
		}

		template<class F> constexpr optional or_else(F&& f) const& { return *this ? *this : std::forward<F>(f)(); }

		template<class F> constexpr optional or_else(F&& f) && { return *this ? move(*this) : std::forward<F>(f)(); }

		optional& operator=(const optional&) = default;
	};

	template<class T> optional(T) -> optional<T>;

	template<class T> constexpr optional<T> make_optional(T && value) {
		return optional<T>(std::forward<T>(value));
	}

	template<class T, class... Args> constexpr optional<T> make_optional(Args && ...args) {
		return optional<T>(std::in_place, std::forward<Args>(args)...);
	}
HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_OPTIONAL_HPP
