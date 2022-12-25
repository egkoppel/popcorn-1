
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_ARCH_AMD64_CONSTANTS_HPP
#define HUGOS_KERNEL_SRC_ARCH_AMD64_CONSTANTS_HPP

namespace memory::constants {
	inline constexpr auto userspace_end               = 0x0000'8000'0000'0000;
	inline constexpr auto mem_map_start               = 0xffff'8000'0000'0000;
	inline constexpr auto page_offset_start           = 0xffff'9000'0000'0000;
	inline constexpr auto page_offset_end             = 0xffff'd000'0000'0000;
	inline constexpr auto kernel_page_allocator_start = 0xffff'd000'0000'0000;
	inline constexpr auto kernel_page_allocator_end   = 0xffff'd700'0000'0000;
	inline constexpr auto kernel_heap_start           = 0xffff'd700'0000'0000;
	inline constexpr auto kernel_heap_end             = 0xffff'de00'0000'0000;
	inline constexpr auto kexe_start                  = 0xffff'ffff'8000'0000;
	inline constexpr auto frame_size                  = 0x1000;
}   // namespace memory::constants

#endif   //HUGOS_KERNEL_SRC_ARCH_AMD64_CONSTANTS_HPP
