
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_CSTDDEF_HPP
#define HUGOS_KERNEL_SRC_STL__STL_CSTDDEF_HPP

#include <stddef.h>

HUGOS_STL_BEGIN_NAMESPACE

using ::size_t;

HUGOS_STL_END_NAMESPACE

#include <type_traits>

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct is_integral;

enum class byte : unsigned char {};

template<class IT> requires(std::is_integral<IT>::value) constexpr IT to_integer(std::byte b) noexcept { return IT(b); }
constexpr byte operator&(byte lhs, byte rhs) noexcept {
	return byte(
        	static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs)
	);
}
constexpr byte operator|(byte lhs, byte rhs) noexcept {
        return byte(
                static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs)
        );
}
constexpr byte operator^(byte lhs, byte rhs) noexcept {
        return byte(
                static_cast<unsigned char>(lhs) ^ static_cast<unsigned char>(rhs)
        );
}
constexpr byte operator~(byte b) noexcept { return byte(~static_cast<unsigned char>(b)); }

constexpr byte& operator&=(byte& lhs, byte rhs) noexcept { return lhs = lhs & rhs; }
constexpr byte& operator|=(byte& lhs, byte rhs) noexcept { return lhs = lhs | rhs; }
constexpr byte& operator^=(byte& lhs, byte rhs) noexcept { return lhs = lhs ^ rhs; }

template<class IT> requires(std::is_integral<IT>::value) constexpr byte operator<<(byte lhs, IT rhs) noexcept { return byte(static_cast<unsigned char>(lhs) << rhs); }
template<class IT> requires(std::is_integral<IT>::value) constexpr byte operator>>(byte lhs, IT rhs) noexcept { return byte(static_cast<unsigned char>(lhs) >> rhs); }

template<class IT> requires(std::is_integral<IT>::value) constexpr byte& operator<<=(byte& lhs, IT rhs) noexcept { return lhs = lhs << rhs; }
template<class IT> requires(std::is_integral<IT>::value) constexpr byte& operator>>=(byte& lhs, IT rhs) noexcept { return lhs = lhs >> rhs; }

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_CSTDDEF_HPP
