
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_TYPE_PROPERTY_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_TYPE_PROPERTY_HPP

#include "integral_constant.hpp"

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct is_const : false_type {};
template<class T> struct is_const<const T> : true_type {};

template<class T> struct is_volatile : false_type {};
template<class T> struct is_volatile<volatile T> : true_type {};

template<class T> inline constexpr auto is_const_v    = is_const<T>::value;
template<class T> inline constexpr auto is_volatile_v = is_volatile<T>::value;

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_TYPE_PROPERTY_HPP
