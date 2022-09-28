/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_MEMORY_H
#define _HUGOS_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint64_t kernel_end;
	uint64_t current_break;
	bool initialised;
} sbrk_state_t;

extern sbrk_state_t global_sbrk_state;

void *sbrk(intptr_t increment); 

#ifdef __cplusplus
}
#endif

#endif
