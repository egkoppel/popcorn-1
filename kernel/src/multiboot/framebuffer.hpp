
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_FRAMEBUFFER_HPP
#define HUGOS_FRAMEBUFFER_HPP

#include "memory/types.hpp"
#include "multiboot.hpp"

namespace multiboot::tags {
	class [[gnu::packed]] Framebuffer : public Tag {
	private:
		memory::paddr_t addr;
		uint32_t pitch;
		[[maybe_unused]] uint32_t width;
		uint32_t height;
		[[maybe_unused]] uint8_t bpp;
		[[maybe_unused]] uint8_t type;

		[[maybe_unused]] uint8_t _0;

		[[maybe_unused]] union {
			struct [[gnu::packed]] {
				uint32_t num_colors;
			} indexed;

			struct [[gnu::packed]] {
				uint8_t red_pos;
				uint8_t red_mask_size;
				uint8_t green_pos;
				uint8_t green_mask_size;
				uint8_t blue_pos;
				uint8_t blue_mask_size;
			} rgb;
		} color_info;

	public:
		memory::paddr_t begin() const { return this->addr; }
		size_t size() const { return (size_t)this->height * (size_t)this->pitch; }
		memory::paddr_t end() const { return this->begin() + this->size(); }
	};
}   // namespace multiboot::tags

#endif   //HUGOS_FRAMEBUFFER_HPP
