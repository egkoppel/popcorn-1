/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUG_MALLOC_TEST_SBRK_H
#define _HUG_MALLOC_TEST_SBRK_H

#include <stddef.h>
#include <stdint.h>

extern char *heap;
extern intptr_t offset;
extern size_t heapsize;

void *sbrk(intptr_t increment);

#endif
