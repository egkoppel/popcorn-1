/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_ALLOCATOR_H
#define _HUGOS_ALLOCATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _allocator_vtable {
	uint64_t (*allocate)(struct _allocator_vtable *);
	uint64_t (*allocate_at)(struct _allocator_vtable *, uint64_t);
	void (*deallocate)(struct _allocator_vtable *, uint64_t);
} allocator_vtable;

uint64_t allocator_allocate(allocator_vtable *);
uint64_t allocator_allocate_at(allocator_vtable *, uint64_t);
void allocator_deallocate(allocator_vtable *, uint64_t);

#ifdef __cplusplus
}
#endif

#endif
