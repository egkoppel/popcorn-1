
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_TYPE_CATEGORY_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_TYPE_CATEGORY_HPP

#include "integral_constant.hpp"

#include <cstddef>

HUGOS_STL_BEGIN_NAMESPACE

	template<class> struct remove_cvref;
	template<class, class> struct is_same;
	template<class> struct is_const;
	template<class> struct is_reference;

	template<class T> struct is_void : bool_constant<__is_void(T)> {};
	template<class T>
	struct is_null_pointer : bool_constant<is_same<decltype(nullptr), typename remove_cvref<T>::type>::value> {};
	template<class T> struct is_integral : bool_constant<__is_integral(T)> {};

	template<class T> struct is_pointer : false_type {};
	template<class T> struct is_pointer<T *> : true_type {};
	template<class T> struct is_pointer<const T *> : true_type {};
	template<class T> struct is_pointer<volatile T *> : true_type {};
	template<class T> struct is_pointer<const volatile T *> : true_type {};

	template<class T> struct is_array : false_type {};
	template<class T> struct is_array<T[]> : true_type {};
	template<class T, std::size_t N> struct is_array<T[N]> : true_type {};

	template<class T> struct is_enum : bool_constant<__is_enum(T)> {};
	template<class T> struct is_union : bool_constant<__is_union(T)> {};
	template<class T> struct is_class : bool_constant<__is_class(T)> {};

	template<class T> struct is_function : bool_constant<!is_const<const T>::value && !is_reference<T>::value> {};

	template<class T> struct is_lvalue_reference : false_type {};
	template<class T> struct is_lvalue_reference<T&> : true_type {};

	template<class T> struct is_rvalue_reference : false_type {};
	template<class T> struct is_rvalue_reference<T&&> : true_type {};

	template<class T> struct is_reference : false_type {};
	template<class T> struct is_reference<T&> : true_type {};
	template<class T> struct is_reference<T&&> : true_type {};

	template<class T> struct is_empty : public bool_constant<__is_empty(T)> {};

	template<class T> inline constexpr auto is_void_v             = is_void<T>::value;
	template<class T> inline constexpr auto is_null_pointer_v     = is_null_pointer<T>::value;
	template<class T> inline constexpr auto is_integral_v         = is_integral<T>::value;
	template<class T> inline constexpr auto is_pointer_v          = is_pointer<T>::value;
	template<class T> inline constexpr auto is_array_v            = is_array<T>::value;
	template<class T> inline constexpr auto is_enum_v             = is_enum<T>::value;
	template<class T> inline constexpr auto is_union_v            = is_union<T>::value;
	template<class T> inline constexpr auto is_class_v            = is_class<T>::value;
	template<class T> inline constexpr auto is_function_v         = is_function<T>::value;
	template<class T> inline constexpr auto is_lvalue_reference_v = is_lvalue_reference<T>::value;
	template<class T> inline constexpr auto is_rvalue_reference_v = is_rvalue_reference<T>::value;
	template<class T> inline constexpr auto is_reference_v        = is_reference<T>::value;
	template<class T> inline constexpr bool is_empty_v            = is_empty<T>::value;

HUGOS_STL_END_NAMESPACE

#endif   // POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_TYPE_CATEGORY_HPP
