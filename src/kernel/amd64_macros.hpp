/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_AMD64_MACROS_HPP
#define HUG_AMD64_MACROS_HPP

#include <stdio.h>

extern inline void sti() {
	fprintf(stdserial, "sti()");
	__asm__ volatile("sti");
}

extern inline void cli() {
	fprintf(stdserial, "cli()");
	__asm__ volatile("cli");
}

extern inline void hlt() {
	fprintf(stdserial, "hlt()");
	__asm__ volatile("hlt");
}

#endif //HUG_AMD64_MACROS_HPP