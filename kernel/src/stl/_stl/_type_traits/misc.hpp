
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_MISC_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_MISC_HPP

#include "cv.hpp"
#include "reference_pointer.hpp"
#include "type_category.hpp"

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct is_array;
template<class T> struct remove_extent;

template<class T> struct remove_cvref {
	typedef remove_reference_t<remove_cv_t<T>> type;
};

template<bool Condition, class T, class F> struct conditional;
template<class T, class F> struct conditional<true, T, F> {
	typedef T type;
};
template<class T, class F> struct conditional<false, T, F> {
	typedef F type;
};

template<class T> struct decay {
private:
	using T_unref = remove_reference_t<T>;

public:
	typedef typename conditional<
			is_array<T_unref>::value,
			add_pointer_t<typename remove_extent<T_unref>::type>,
			typename conditional<is_function<T_unref>::value, add_pointer_t<T_unref>, remove_cv_t<T_unref>>::type>::type
			type;
};

template<bool B, class T> struct enable_if {};
template<class T> struct enable_if<true, T> {
	typedef T type;
};

template<class T> struct underlying_type {
	typedef __underlying_type(T) type;
};

template<class T> using remove_cvref_t                 = typename remove_cvref<T>::type;
template<bool B, class T, class F> using conditional_t = typename conditional<B, T, F>::type;
template<class T> using decay_t                        = typename decay<T>::type;
template<bool B, class T = void> using enable_if_t     = typename enable_if<B, T>::type;
template<class T> using underlying_type_t              = typename underlying_type<T>::type;

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_MISC_HPP
