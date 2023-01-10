/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_UTILS_H
#define _HUGOS_UTILS_H

#include <panic.h>

#ifdef __cplusplus

	#include <utility>

	#define ALIGN_UP(ptr, alignment)   ((decltype(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
	#define ALIGN_DOWN(ptr, alignment) ((decltype(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
	#define ADD_BYTES(ptr, offset)     ((decltype(ptr))((uintptr_t)ptr + offset))

	#define IDIV_ROUND_UP(n, d)        (((n) + (d)-1) / (d))

#else
	#define ALIGN_UP(ptr, alignment)   ((typeof(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
	#define ALIGN_DOWN(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
	#define ADD_BYTES(ptr, offset)     ((typeof(ptr))((uintptr_t)ptr + offset))

	#define IDIV_ROUND_UP(n, d)        (((n) + (d)-1) / (d))

#endif

#endif
