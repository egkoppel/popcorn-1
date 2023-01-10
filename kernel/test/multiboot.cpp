/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <stdint.h>

#define private   public
#define protected public

#include <multiboot/boot_module.hpp>
#include <multiboot/bootloader.hpp>
#include <multiboot/cli.hpp>
#include <multiboot/elf_sections.hpp>
#include <multiboot/framebuffer.hpp>
#include <multiboot/memory_map.hpp>
#include <multiboot/multiboot.hpp>
#include <multiboot/rsdp.hpp>

#undef private
#undef protected

TEST(MultibootInfoParsing, InfoHeaderSizeParse) {
	char mem_data_size0[] = {8, 0, 0, 0, 0, 0, 0, 0};
	multiboot::Data multiboot_data_size0(reinterpret_cast<uint64_t>(mem_data_size0));
	EXPECT_EQ(multiboot_data_size0.mb_data_start, multiboot_data_size0.mb_data_end)
			<< "First and last tag address are not the same when header is size 0";

	uint32_t mem_data_size10[] = {18, 0, 0, 0, 0, 0, 0, 0};
	multiboot::Data multiboot_data_size10(reinterpret_cast<uint64_t>(mem_data_size10));
	EXPECT_EQ(((char *)multiboot_data_size10.mb_data_start) + 10, (char *)multiboot_data_size10.mb_data_end)
			<< "First and last tag address are not 10 bytes apart";
}

TEST(MultibootInfoParsing, LocateTags) {
	char mem_data[] = {
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0, /* Info header */
			(char)multiboot::TagType::CLI,
			0,
			0,
			0,
			8,
			0,
			0,
			0, /* CLI tag */
			(char)multiboot::TagType::MEMORY_MAP,
			0,
			0,
			0,
			8,
			0,
			0,
			0 /* Memory map tag */
	};
	mem_data[0] = sizeof(mem_data);
	multiboot::Data multiboot_data(reinterpret_cast<uint64_t>(mem_data));

	auto cli_tag = multiboot_data.find_tag<char>(multiboot::TagType::CLI);
	ASSERT_TRUE(!!cli_tag) << "First tag not found";
	EXPECT_EQ(cli_tag.value(), &mem_data[8]) << "First tag not pointing to start of first tag";

	auto mmap_tag = multiboot_data.find_tag<char>(multiboot::TagType::MEMORY_MAP);
	ASSERT_TRUE(!!mmap_tag) << "Second tag not found";
	EXPECT_EQ(mmap_tag.value(), &mem_data[16]) << "Second tag not pointing to start of first tag";

	auto fb_tag = multiboot_data.find_tag<char>(multiboot::TagType::FRAMEBUFFER);
	EXPECT_FALSE(!!fb_tag) << "Claimed non-present tag as found";
	EXPECT_THROW(fb_tag.value(), std::bad_optional_access);
}
