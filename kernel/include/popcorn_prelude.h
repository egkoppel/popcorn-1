
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_INCLUDE_POPCORN_PRELUDE_HPP
#define POPCORN_KERNEL_INCLUDE_POPCORN_PRELUDE_HPP

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned _BitInt(128) u128;
#if BITINT_256_SUPPORT
typedef unsigned _BitInt(256) u256;
#else
	#define u256 static_assert(false, "u256 not supported by this compiler")
#endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef signed _BitInt(128) i128;
#if BITINT_256_SUPPORT
typedef signed _BitInt(256) i256;
#else
	#define i256 static_assert(false, "u256 not supported by this compiler")
#endif

typedef size_t usize;

#define cpu_local thread_local

#ifdef __cplusplus
	#include <log.hpp>

struct deep_copy_t {};
inline constexpr deep_copy_t deep_copy{};

struct shallow_copy_t {};
inline constexpr shallow_copy_t shallow_copy{};

	#define THROW(x)                                                                                                   \
		{                                                                                                              \
			LOG(Log::CRITICAL, "Throwing " #x);                                                                        \
			throw x;                                                                                                   \
		}
#endif
#endif   // POPCORN_KERNEL_INCLUDE_POPCORN_PRELUDE_HPP
