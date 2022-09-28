/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "allocator.h"
#include <stddef.h>

uint64_t allocator_allocate(allocator_vtable *allocator) {
	if (allocator->allocate != NULL) return allocator->allocate(allocator);
	return 0;
}

void allocator_deallocate(allocator_vtable *allocator, uint64_t address) {
	if (allocator->deallocate != NULL) allocator->deallocate(allocator, address);
}
