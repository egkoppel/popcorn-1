
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_HAL_AMD64_HPP
#define HUGOS_HAL_AMD64_HPP

extern inline void enable_interrupts() {
	__asm__ volatile("sti");
}

extern inline void disable_interrupts() {
	__asm__ volatile("cli");
}

extern inline void halt() {
	__asm__ volatile("hlt");
}

extern inline void nop() {
	__asm__ volatile("nop");
}

#endif //HUGOS_HAL_AMD64_HPP
