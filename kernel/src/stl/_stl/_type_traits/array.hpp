
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_ARRAY_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_ARRAY_HPP

#include <cstddef>

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct remove_extent {
	typedef T type;
};
template<class T> struct remove_extent<T[]> {
	typedef T type;
};
template<class T, std::size_t N> struct remove_extent<T[N]> {
	typedef T type;
};

template<class T> struct remove_all_extents {
	typedef T type;
};
template<class T> struct remove_all_extents<T[]> {
	typedef typename remove_all_extents<T>::type type;
};
template<class T, std::size_t N> struct remove_all_extents<T[N]> {
	typedef typename remove_all_extents<T>::type type;
};

template<class T> using remove_extent_t      = typename remove_extent<T>::type;
template<class T> using remove_all_extents_t = typename remove_all_extents<T>::type;

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_ARRAY_HPP
