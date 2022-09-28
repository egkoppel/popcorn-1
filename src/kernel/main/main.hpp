/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_MAIN_H
#define _HUGOS_MAIN_H

#include "../memory/allocator.h"
#include "../gdt/gdt.hpp"
#include "../gdt/tss.hpp"

extern "C" {

extern allocator_vtable *global_frame_allocator;
extern gdt::GDT global_descriptor_table;
extern tss::TSS task_state_segment;

}

#endif
