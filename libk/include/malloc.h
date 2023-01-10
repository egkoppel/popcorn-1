/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUG_MALLOC_H
#define _HUG_MALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct header {
	size_t size;
	struct header *prev_free, *next_free;
	bool is_free;
	uint32_t pad;
} Header;

typedef struct {
	Header *header;
	uint64_t pad;
} Footer;

enum free_options {
	NORMAL = 0,
	ONLY_MERGE_FORWARDS = 1,
	NO_RETURN_MEMORY = 2,
};

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void *ptr, size_t new_size);
void free(void *ptr);

Header* __hug_malloc_get_first_free();
void __hug_malloc_clear_first_free();
void __hug_malloc_set_first_malloc();

#ifdef __cplusplus
}
#endif

#endif
