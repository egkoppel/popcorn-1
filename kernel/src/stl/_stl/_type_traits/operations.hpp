
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_OPERATIONS_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_OPERATIONS_HPP

#include "cv.hpp"
#include "integral_constant.hpp"
#include "reference_pointer.hpp"

HUGOS_STL_BEGIN_NAMESPACE

template<class T, class... Args> struct is_constructible : bool_constant<__is_constructible(T, Args...)> {};
template<class T, class... Args>
struct is_trivially_constructible : bool_constant<__is_trivially_constructible(T, Args...)> {};
template<class T, class... Args>
struct is_nothrow_constructible : bool_constant<__is_nothrow_constructible(T, Args...)> {};

template<class T> struct is_default_constructible : is_constructible<T> {};
template<class T> struct is_trivially_default_constructible : is_trivially_constructible<T> {};
template<class T> struct is_nothrow_default_constructible : is_nothrow_constructible<T> {};

template<class T> struct is_copy_constructible : is_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};
template<class T>
struct is_trivially_copy_constructible : is_trivially_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};
template<class T>
struct is_nothrow_copy_constructible : is_nothrow_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};

template<class T> struct is_move_constructible : is_constructible<T, add_rvalue_reference_t<T>> {};
template<class T> struct is_trivially_move_constructible : is_trivially_constructible<T, add_rvalue_reference_t<T>> {};
template<class T> struct is_nothrow_move_constructible : is_nothrow_constructible<T, add_rvalue_reference_t<T>> {};

template<class T> struct is_trivially_copyable : bool_constant<__is_trivially_copyable(T)> {};

template<class T, class... Args> inline constexpr auto is_constructible_v = is_constructible<T, Args...>::value;
template<class T, class... Args>
inline constexpr auto is_trivially_constructible_v = is_trivially_constructible<T, Args...>::value;
template<class T, class... Args>
inline constexpr auto is_nothrow_constructible_v                   = is_nothrow_constructible<T, Args...>::value;
template<class T> inline constexpr auto is_default_constructible_v = is_default_constructible<T>::value;
template<class T>
inline constexpr auto is_trivially_default_constructible_v = is_trivially_default_constructible<T>::value;
template<class T> inline constexpr auto is_nothrow_default_constructible_v = is_nothrow_default_constructible<T>::value;
template<class T> inline constexpr auto is_copy_constructible_v            = is_copy_constructible<T>::value;
template<class T> inline constexpr auto is_trivially_copy_constructible_v  = is_trivially_copy_constructible<T>::value;
template<class T> inline constexpr auto is_nothrow_copy_constructible_v    = is_nothrow_copy_constructible<T>::value;
template<class T> inline constexpr auto is_move_constructible_v            = is_move_constructible<T>::value;
template<class T> inline constexpr auto is_trivially_move_constructible_v  = is_trivially_move_constructible<T>::value;
template<class T> inline constexpr auto is_nothrow_move_constructible_v    = is_nothrow_move_constructible<T>::value;
template<class T> inline constexpr auto is_trivially_copyable_v            = is_trivially_copyable<T>::value;

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_OPERATIONS_HPP
