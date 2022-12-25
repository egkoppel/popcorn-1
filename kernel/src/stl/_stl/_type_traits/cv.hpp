
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_CV_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_CV_HPP

#include "integral_constant.hpp"

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct remove_const {
	typedef T type;
};
template<class T> struct remove_const<const T> {
	typedef T type;
};

template<class T> struct remove_volatile {
	typedef T type;
};
template<class T> struct remove_volatile<volatile T> {
	typedef T type;
};

template<class T> struct remove_cv {
	typedef typename remove_const<typename remove_volatile<T>::type>::type type;
};

template<class T> struct add_const {
	typedef const T type;
};

template<class T> struct add_volatile {
	typedef volatile T type;
};

template<class T> struct add_cv {
	typedef typename add_const<typename add_volatile<T>::type>::type type;
};

template<class T> using remove_const_t    = typename remove_const<T>::type;
template<class T> using remove_volatile_t = typename remove_volatile<T>::type;
template<class T> using remove_cv_t       = typename remove_cv<T>::type;
template<class T> using add_const_t       = typename add_const<T>::type;
template<class T> using add_volatile_t    = typename add_volatile<T>::type;
template<class T> using add_cv_t          = typename add_cv<T>::type;

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_CV_HPP
