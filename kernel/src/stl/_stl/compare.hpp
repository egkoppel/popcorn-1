
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

#ifndef HUGOS_KERNEL_SRC_STL__STL_COMPARE_HPP
#define HUGOS_KERNEL_SRC_STL__STL_COMPARE_HPP

HUGOS_STL_BEGIN_NAMESPACE
class strong_ordering {
public:
	enum state_t { LESS, GREATER, EQUAL, EQUIVALENT };

private:
	state_t state;

public:
	constexpr explicit strong_ordering(state_t s) : state(s) {}
	static const strong_ordering less;
	static const strong_ordering greater;
	static const strong_ordering equal;
	static const strong_ordering equivalent;

	friend constexpr bool operator==(strong_ordering, strong_ordering) = default;
	friend constexpr bool operator<(strong_ordering lhs, int) { return lhs.state == LESS; }
	friend constexpr bool operator>(strong_ordering lhs, int) { return lhs.state == GREATER; }
	friend constexpr bool operator<=(strong_ordering lhs, int) {
		return lhs.state == LESS || lhs.state == EQUAL || lhs.state == EQUIVALENT;
	}
	friend constexpr bool operator>=(strong_ordering lhs, int) {
		return lhs.state == GREATER || lhs.state == EQUAL || lhs.state == EQUIVALENT;
	}
	friend constexpr bool operator==(strong_ordering lhs, int) { return lhs.state == EQUAL || lhs.state == EQUIVALENT; }
};

inline constexpr strong_ordering strong_ordering::less(strong_ordering::LESS);
inline constexpr strong_ordering strong_ordering::greater(strong_ordering::GREATER);
inline constexpr strong_ordering strong_ordering::equal(strong_ordering::EQUAL);
inline constexpr strong_ordering strong_ordering::equivalent(strong_ordering::EQUIVALENT);

class weak_ordering {
public:
	enum state_t { LESS, GREATER, EQUAL, EQUIVALENT };

private:
	state_t state;

public:
	constexpr explicit weak_ordering(state_t s) : state(s) {}
	static const weak_ordering less;
	static const weak_ordering greater;
	static const weak_ordering equal;
	static const weak_ordering equivalent;

	friend constexpr bool operator==(weak_ordering, weak_ordering) = default;
	friend constexpr bool operator<(weak_ordering lhs, int) { return lhs.state == LESS; }
	friend constexpr bool operator>(weak_ordering lhs, int) { return lhs.state == GREATER; }
	friend constexpr bool operator<=(weak_ordering lhs, int) {
		return lhs.state == LESS || lhs.state == EQUAL || lhs.state == EQUIVALENT;
	}
	friend constexpr bool operator>=(weak_ordering lhs, int) {
		return lhs.state == GREATER || lhs.state == EQUAL || lhs.state == EQUIVALENT;
	}
	friend constexpr bool operator==(weak_ordering lhs, int) { return lhs.state == EQUAL || lhs.state == EQUIVALENT; }
};

inline constexpr weak_ordering weak_ordering::less(weak_ordering::LESS);
inline constexpr weak_ordering weak_ordering::greater(weak_ordering::GREATER);
inline constexpr weak_ordering weak_ordering::equal(weak_ordering::EQUAL);
inline constexpr weak_ordering weak_ordering::equivalent(weak_ordering::EQUIVALENT);
HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_COMPARE_HPP
