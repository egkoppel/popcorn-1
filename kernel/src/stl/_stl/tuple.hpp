
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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

#ifndef HUGOS_KERNEL_SRC_STL__STL_TUPLE_HPP
#define HUGOS_KERNEL_SRC_STL__STL_TUPLE_HPP

#include <stddef.h>
HUGOS_STL_BEGIN_NAMESPACE
namespace detail {
	template<size_t Index, typename T> class tuple_leaf {
	public:
		T value;
		tuple_leaf(const T& value) : value(value) {}
	};

	template<class s, class... Ts> class tuple_impl {};

	template<size_t... Is, class... Ts>
	class tuple_impl<std::index_sequence<Is...>, Ts...> : public tuple_leaf<Is, Ts>... {
	public:
		tuple_impl(const Ts&...args) : tuple_leaf<Is, Ts>(args)... {}
	};
}   // namespace detail

template<class... Types> class tuple;

template<class... Types> tuple<typename std::decay<Types>::type...> make_tuple(Types&&...args) { return {args...}; }

template<class T> struct tuple_size;
template<class... Types> struct tuple_size<tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template<size_t I, class T> struct tuple_element;
template<size_t I, class Head, class... Rest>
struct tuple_element<I, tuple<Head, Rest...>> : public tuple_element<I - 1, tuple<Rest...>> {};
template<class Head, class... Rest> struct tuple_element<0, tuple<Head, Rest...>> {
	using type = Head;
};

template<class... Types> class tuple {
	template<size_t I> friend typename tuple_element<I, tuple<Types...>>::type& get(tuple<Types...>& t);
	template<size_t I> friend typename tuple_element<I, tuple<Types...>>::type&& get(tuple<Types...>&& t);
	template<size_t I> friend const typename tuple_element<I, tuple<Types...>>::type& get(const tuple<Types...>& t);
	template<size_t I> friend const typename tuple_element<I, tuple<Types...>>::type&& get(const tuple<Types...>&& t);

private:
	detail::tuple_impl<std::make_index_sequence<sizeof...(Types)>, Types...> impl;

public:
	constexpr tuple(const Types&...args) : impl(args...) {}
};

template<size_t I, class... Types> typename tuple_element<I, tuple<Types...>>::type& get(tuple<Types...>& t) noexcept {
	return static_cast<detail::tuple_leaf<I, typename tuple_element<I, tuple<Types...>>::type>>(t.impl).value;
}
template<size_t I, class... Types> typename tuple_element<I, tuple<Types...>>::type&& get(tuple<Types...>&& t) {
	return static_cast<detail::tuple_leaf<I, typename tuple_element<I, tuple<Types...>>::type>>(t.impl).value;
}
template<size_t I, class... Types>
const typename tuple_element<I, tuple<Types...>>::type& get(const tuple<Types...>& t) {
	return static_cast<detail::tuple_leaf<I, typename tuple_element<I, tuple<Types...>>::type>>(t.impl).value;
}
template<size_t I, class... Types>
const typename tuple_element<I, tuple<Types...>>::type&& get(const tuple<Types...>&& t) {
	return static_cast<detail::tuple_leaf<I, typename tuple_element<I, tuple<Types...>>::type>>(t.impl).value;
}

namespace detail {
	struct ignore_t {
		template<typename T> constexpr void operator=(T&&) const noexcept {}
	};
}   // namespace detail
inline constexpr detail::ignore_t ignore;

template<class Fp, class... Args>
inline constexpr decltype(std::declval<Fp>()(std::declval<Args>()...))
invoke(Fp&& f, Args&&...args) noexcept(noexcept(static_cast<Fp&&>(f)(static_cast<Args&&>(args)...)));

namespace detail {
	template<class F, class Tuple, std::size_t... Ns>
	constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, index_sequence<Ns...>) noexcept(
			noexcept(std::invoke(std::forward<F>(f), get<Ns>(std::forward<Tuple>(t))...))) {
		return std::invoke(std::forward<F>(f), get<Ns>(std::forward<Tuple>(t))...);
	}
}   // namespace detail

template<class F, class Tuple> constexpr decltype(auto) apply(F&& f, Tuple&& t) noexcept(noexcept(
		detail::apply_impl(std::forward<F>(f),
                           std::forward<Tuple>(t),
                           make_index_sequence<tuple_size<Tuple>::value>())
		)) {
	return detail::apply_impl(std::forward<F>(f),
	                          std::forward<Tuple>(t),
	                          make_index_sequence<tuple_size<Tuple>::value>());
}

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_TUPLE_HPP
