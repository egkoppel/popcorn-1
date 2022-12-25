
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

#ifndef POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_INTEGRAL_CONSTANT_HPP
#define POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_INTEGRAL_CONSTANT_HPP

HUGOS_STL_BEGIN_NAMESPACE
template<class T, T v> struct integral_constant {
	static constexpr T value = v;
};
template<bool B> using bool_constant = integral_constant<bool, B>;
using true_type                      = integral_constant<bool, true>;
using false_type                     = integral_constant<bool, false>;
HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL__TYPE_TRAITS_INTEGRAL_CONSTANT_HPP
