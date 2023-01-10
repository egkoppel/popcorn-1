
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_FRAMEBUFFER_HPP
#define HUGOS_FRAMEBUFFER_HPP

#include <memory/types.hpp>
#include <popcorn_prelude.h>
#include "multiboot.hpp"

namespace multiboot::tags {
	class [[gnu::packed]] Framebuffer : public Tag {
	private:
		memory::paddr_t addr;
		u32 pitch;
		[[maybe_unused]] u32 width;
		u32 height;
		[[maybe_unused]] u8 bpp;
		[[maybe_unused]] u8 type;

		[[maybe_unused]] u8 _0;

		[[maybe_unused]] union {
			struct [[gnu::packed]] {
				u32 num_colors;
			} indexed;

			struct [[gnu::packed]] {
				u8 red_pos;
				u8 red_mask_size;
				u8 green_pos;
				u8 green_mask_size;
				u8 blue_pos;
				u8 blue_mask_size;
			} rgb;
		} color_info;

	public:
		memory::paddr_t begin() const { return this->addr; }
		size_t size() const { return (size_t)this->height * (size_t)this->pitch; }
		memory::paddr_t end() const { return this->begin() + this->size(); }
	};
}   // namespace multiboot::tags

#endif   //HUGOS_FRAMEBUFFER_HPP
