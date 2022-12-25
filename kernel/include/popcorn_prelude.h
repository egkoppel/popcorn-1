
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;

#define cpu_local thread_local

#ifdef __cplusplus
struct deep_copy_t {};
inline constexpr deep_copy_t deep_copy{};

struct shallow_copy_t {};
inline constexpr shallow_copy_t shallow_copy{};
#endif
#endif   //POPCORN_KERNEL_INCLUDE_POPCORN_PRELUDE_HPP
