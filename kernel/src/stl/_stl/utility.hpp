
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

#ifndef HUGOS_KERNEL_SRC_STL__STL_UTILITY_HPP
#define HUGOS_KERNEL_SRC_STL__STL_UTILITY_HPP

#include <compare>
#include <stddef.h>
#include <type_traits>

HUGOS_STL_BEGIN_NAMESPACE
template<class T> constexpr const T& as_const(T& t) noexcept { return t; }

template<class T> typename std::remove_reference<T>::type&& move(T&& t) noexcept {
	return static_cast<typename std::remove_reference<T>::type&&>(t);
}

template<class T> T&& forward(typename std::remove_reference<T>::type& t) noexcept { return static_cast<T&&>(t); }

template<class T> T&& forward(typename std::remove_reference<T>::type&& t) noexcept { return static_cast<T&&>(t); }

template<class T1, class T2> struct pair {
	[[no_unique_address]] T1 first;
	[[no_unique_address]] T2 second;

	pair(const pair&) = default;
	pair(pair&&)      = default;

	template<class U1 = T1, class U2 = T2>
	pair(U1&& u1, U2&& u2) noexcept(std::is_nothrow_constructible_v<T1, U1>&& std::is_nothrow_constructible_v<T1, U2>) :
		first(forward<U1>(u1)),
		second(forward<U2>(u2)) {}
	template<class U1, class U2>
	pair(const pair<U1, U2>& p) noexcept(
			std::is_nothrow_constructible_v<T1, U1&>&& std::is_nothrow_constructible_v<T1, U2&>) :
		first(p.first),
		second(p.second) {}

	template<class U1, class U2>
	explicit pair(pair<U1, U2>&& p) noexcept(
			std::is_nothrow_constructible_v<T1, U1&&>&& std::is_nothrow_constructible_v<T1, U2&&>) :
		first(forward<U1>(p.first)),
		second(forward<U2>(p.second)) {}
};

template<class T1, class T2>
constexpr std::weak_ordering
operator<=>(const std::pair<T1, T2>& lhs,
            const std::pair<T1, T2>& rhs) noexcept(noexcept(lhs < rhs) && noexcept(rhs < lhs)) {
	return lhs < rhs ? std::weak_ordering::less :
	       rhs < lhs ? std::weak_ordering::greater :
	                   std::weak_ordering::equivalent;
}

template<class T1, class T2> pair(T1, T2) -> pair<T1, T2>;

template<class T1, class T2>
constexpr auto make_pair(T1&& t, T2&& u) noexcept(noexcept(pair<T1, T2>(forward<T1>(t), forward<T2>(u)))) {
	return pair</*typename decay<T1>::type, typename decay<T2>::type*/ T1, T2>(forward<T1>(t), forward<T2>(u));
}

template<class T, T... Ints> class integer_sequence {
	static constexpr size_t size() noexcept { return sizeof...(Ints); }
};

template<size_t... Ints> using index_sequence      = integer_sequence<size_t, Ints...>;
template<class T, T N> using make_integer_sequence = __make_integer_seq<integer_sequence, T, N>;
template<size_t N> using make_index_sequence       = make_integer_sequence<size_t, N>;

struct in_place_t {
	explicit in_place_t() noexcept = default;
};

template<class T> struct in_place_type_t {
	explicit in_place_type_t() noexcept = default;
};

template<size_t I> struct in_place_index_t {
	explicit in_place_index_t() noexcept = default;
};

inline constexpr in_place_t in_place{};
template<class T> inline constexpr in_place_type_t<T> in_place_type{};
template<size_t I> inline constexpr in_place_index_t<I> in_place_index{};

template<typename T> constexpr bool __always_false = false;
template<typename T> typename add_rvalue_reference<T>::type declval() noexcept {
	static_assert(__always_false<T>, "declval not allowed in an evaluated context");
}

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_UTILITY_HPP
