/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_ASSERT_H
#define _HUGOS_ASSERT_H


#ifdef NDEBUG
	#define assert(x)                                                                                                  \
		{}
	#define assert_msg(x, s, ...)                                                                                      \
		{}
#else
	#ifndef HUGOS_TEST
		#include <panic.h>

		#define assert(x)                                                                                              \
			{                                                                                                          \
				if (!(x)) panic("Assertion failed: " #x "\n");                                                         \
			}
	#else
		#include_next <assert.h>
	#endif

	#define assert_msg(x, s, ...)                                                                                      \
		{                                                                                                              \
			assert(x);                                                                                                 \
			if (!(x)) printf(s, ##__VA_ARGS__);                                                                        \
		}
#endif

#ifndef __cplusplus
	#define STATIC_ASSERT(cond) typedef char __static_assert_type[(cond) ? 1 : -1];
#endif

#endif
