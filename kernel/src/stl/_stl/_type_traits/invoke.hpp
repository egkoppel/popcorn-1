
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_INVOKE_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_INVOKE_HPP

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct add_rvalue_reference;
template<typename T> typename add_rvalue_reference<T>::type declval() noexcept;
template<class T, class U> struct is_same;
template<bool, class, class> struct conditional;
template<class> struct is_void;
template<bool B, class T = void> struct enable_if;

HUGOS_STL_BEGIN_PRIVATE_NAMESPACE

template<class T, class U, class = void> struct is_core_convertible : false_type {};

template<class T, class U>
struct is_core_convertible<T, U, decltype(static_cast<void (*)(U)>(0)(static_cast<T (*)()>(0)()))> : true_type {};

struct any {
	any(...);
};

struct nat {};

template<class Fp, class... Args>
inline constexpr decltype(std::declval<Fp>()(std::declval<Args>()...))
invoke(Fp&& f, Args&&...args) noexcept(noexcept(static_cast<Fp&&>(f)(static_cast<Args&&>(args)...))) {
	return static_cast<Fp&&>(f)(static_cast<Args&&>(args)...);
}


template<class... Args> nat invoke(any, Args&&...args);

template<class Ret, class Fp, class... Args> struct invokable_r {
	template<class XFp, class... XArgs> static decltype(invoke(declval<XFp>(), declval<XArgs>()...)) __try_call(int);
	template<class XFp, class... XArgs> static nat __try_call(...);

	// FIXME: Check that _Ret, _Fp, and _Args... are all complete types, cv void,
	// or incomplete array types as required by the standard.
	using Result = decltype(__try_call<Fp, Args...>(0));

	using type = typename conditional<
			!is_same<Result, nat>::value,
			typename conditional<is_void<Ret>::value, true_type, is_core_convertible<Result, Ret>>::type,
			false_type>::type;
	static const bool value = type::value;
};
template<class Fp, class... Args> using invokable = invokable_r<void, Fp, Args...>;

template<class Fp, class... Args>
struct invoke_of : enable_if<invokable<Fp, Args...>::value, typename invokable_r<void, Fp, Args...>::Result> {};

HUGOS_STL_END_PRIVATE_NAMESPACE

template<class Fn, class... Args> struct invoke_result : HUGOS_STL_PRIVATE_NAMESPACE::invoke_of<Fn, Args...> {};
template<class Fn, class... Args> using invoke_result_t = typename invoke_result<Fn, Args...>::type;

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_INVOKE_HPP
