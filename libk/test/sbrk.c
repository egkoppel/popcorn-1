/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sbrk.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

char *heap;
intptr_t offset;
size_t heapsize;

void *sbrk(intptr_t increment) {
	intptr_t old_offset = offset;
	intptr_t new_offset = offset + increment;
	if ((unsigned)new_offset >= heapsize) return (void *)-1;
	if (new_offset < 0) return (void *)-1;
	offset = new_offset;
	return (void *)(heap + old_offset);
}
