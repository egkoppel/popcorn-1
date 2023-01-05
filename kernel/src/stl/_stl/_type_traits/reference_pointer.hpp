
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_REFERENCE_POINTER_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_REFERENCE_POINTER_HPP

HUGOS_STL_BEGIN_NAMESPACE

	template<class T> struct remove_pointer {
		typedef T type;
	};

	template<class T> struct remove_pointer<T *> {
		typedef T type;
	};

	template<class T> struct remove_pointer<const T *> {
		typedef T type;
	};

	template<class T> struct remove_pointer<volatile T *> {
		typedef T type;
	};

	template<class T> struct remove_pointer<const volatile T *> {
		typedef T type;
	};

	template<class T> struct remove_reference {
		typedef T type;
	};
	template<class T> struct remove_reference<T&> {
		typedef T type;
	};
	template<class T> struct remove_reference<T&&> {
		typedef T type;
	};

	template<class T> struct add_lvalue_reference {
		typedef T& type;
	};
	template<class T> struct add_lvalue_reference<T&> {
		typedef T& type;
	};
	template<class T> struct add_lvalue_reference<T&&> {
		typedef T& type;
	};

	template<class T> struct add_rvalue_reference {
		typedef T&& type;
	};
	template<class T> struct add_rvalue_reference<T&> {
		typedef T&& type;
	};
	template<class T> struct add_rvalue_reference<T&&> {
		typedef T&& type;
	};

	template<class T> struct add_pointer {
		typedef typename remove_reference<T>::type *type;
	};
	template<class T> struct add_pointer<T *> {
		typedef T *type;
	};

	template<class T> using remove_pointer_t       = typename remove_pointer<T>::type;
	template<class T> using remove_reference_t     = typename remove_reference<T>::type;
	template<class T> using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;
	template<class T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
	template<class T> using add_pointer_t          = typename add_pointer<T>::type;

HUGOS_STL_END_NAMESPACE

#endif   // POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_REFERENCE_POINTER_HPP
