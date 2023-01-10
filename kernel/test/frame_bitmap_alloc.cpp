// Copyright (c) 2022 Eliyahu Gluschove-Koppel.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fmt/core.h>
#include <gtest/gtest.h>

#define private   public
#define protected public

#include <memory/physical_allocators/bitmap_allocator.hpp>

#undef private
#undef protected

using namespace memory;

class BitmapAllocator : public ::testing::Test,
						public testing::WithParamInterface<int> {
private:
	void *start;
	void *do_allocations() {
		this->start = malloc(GetParam() * 8);
		if (!this->start) throw std::bad_alloc();
		for (auto i = reinterpret_cast<uint64_t *>(this->start);
		     i < reinterpret_cast<uint64_t *>((char *)this->start + GetParam() * 8);
		     i++)
			*i = std::rand();
		return this->start;
	}

protected:
	physical_allocators::BitmapAllocator allocator;

	BitmapAllocator() :
		allocator(Frame::containing_address(0_pa),
	              static_cast<uint64_t *>(do_allocations()),
	              reinterpret_cast<uint64_t *>((char *)this->start + GetParam() * 8)) {}

	~BitmapAllocator() override { free(allocator.bitmap_start); }
};

TEST_P(BitmapAllocator, AllocatesAllMemory) {
	for (unsigned int i = 0; i < GetParam() * 64; i++) {
		EXPECT_NO_THROW(auto f = allocator.allocate(0)) << "Failed to allocate frame on iteration " << i + 1;
	}
	EXPECT_THROW(allocator.allocate(0), std::bad_alloc) << "Allocates memory after all memory should be full";
}

TEST_P(BitmapAllocator, AllocatesAllMemoryWithPlacementAlloc) {
	for (unsigned int i = 0; i < GetParam() * 64; i++) {
		auto f = Frame::from_address(PhysicalAddress{i * 0x1000});
		EXPECT_FALSE(allocator.get_bit(f)) << "Failed on iteration " << i + 1;
		//EXPECT_TRUE(allocator.allocate_at(f, 0).is_some()) << "Failed on iteration " << i + 1;
	}
}

TEST_P(BitmapAllocator, FreesAllMemory) {
	for (unsigned int i = 0; i < GetParam() * 64; i++) { allocator.allocate(0); }
	for (unsigned int i = 0; i < GetParam() * 64; i++) {
		allocator.deallocate({Frame::from_address(PhysicalAddress(i * 0x1000))});
	}
	for (unsigned int i = 0; i < GetParam() * 64; i++) { EXPECT_NO_THROW(allocator.allocate(0)); }
}

TEST_P(BitmapAllocator, ErrorOnOom) {
	for (unsigned int i = 0; i < GetParam() * 64; i++) { EXPECT_NO_THROW(allocator.allocate(0)); }
	EXPECT_THROW(allocator.allocate(0), std::bad_alloc);
}

/*TEST_P(BitmapAllocator, ErrorOnPlacementAllocForAlreadyAllocated) {
	for (unsigned int i = 0; i < GetParam() * 64; i++) {
		auto f = allocator.allocate(0);
		ASSERT_TRUE(f.is_some()) << "Failed to allocate frame on iteration " << i + 1;
		EXPECT_FALSE(allocator.allocate_at(f.unwrap(), 0).is_some())
				<< "Placement allocated frame at 0x" << std::hex << f.unwrap().begin().address() << std::dec;
	}
}*/

TEST_P(BitmapAllocator, ReturnsDifferentMemory) {
	std::vector<Frame> frames;
	try {
		auto f = *allocator.allocate(0).begin();
		while (true) {
			EXPECT_EQ(std::find(frames.begin(), frames.end(), f), frames.end()) << "Frame was repeated";
			frames.push_back(f);
			f = *allocator.allocate(0).begin();
		}
	} catch (std::bad_alloc&) {}
}

/*TEST_P(BitmapAllocator, ErrorOnPlacementAllocOutsideOfMemory) {
	for (unsigned int i = GetParam() * 64; i < GetParam() * 64 + 64; i++) {
		auto f = allocator.allocate_at(Frame::from_address(PhysicalAddress{i * 0x1000}), 0);
		EXPECT_FALSE(f.is_some());
	}
}*/

TEST_P(BitmapAllocator, JunkMemoryZeroed) {
	for (unsigned int i = 0; i < GetParam() * 64; i++) {
		EXPECT_FALSE(allocator.get_bit(Frame::from_address(PhysicalAddress(i * 0x1000))))
				<< "Failed on iteration " << i + 1;
	}
}

INSTANTIATE_TEST_SUITE_P(VaryingMemorySize,
                         BitmapAllocator,
                         testing::Range(0, 16),
                         [](const testing::TestParamInfo<BitmapAllocator::ParamType>& info) {
							 return fmt::format("{}KiB_memory", info.param * 64 * 4);
						 });

TEST(BitmapAllocator, InstantiateFromBumpAllocator) {
	struct MemoryMapData {
		multiboot::tags::MemoryMap tag;
		multiboot::tags::MemoryMap::Entry other_entries[2];
	} __attribute__((packed));

	MemoryMapData memory_map;
	physical_allocators::MonotonicAllocator bump_allocator(PhysicalAddress{0},
	                                                       PhysicalAddress{0x1000},
	                                                       PhysicalAddress{0x5fff},
	                                                       PhysicalAddress{0x6000},
	                                                       PhysicalAddress{0xffff},
	                                                       &memory_map.tag);

	memory_map.tag.type          = multiboot::TagType::MEMORY_MAP;
	memory_map.tag.size          = sizeof(memory_map);
	memory_map.tag.entry_size    = sizeof(multiboot::tags::MemoryMap::Entry);
	memory_map.tag.entry_version = 0;
	memory_map.tag.first_entry   = {.base_addr = PhysicalAddress(0),
	                                .length    = 0x10000,
	                                .type      = multiboot::tags::MemoryMap::Type::AVAILABLE,
	                                .reserved  = 0};
	memory_map.other_entries[0]  = {.base_addr = PhysicalAddress(0x10000),
	                                .length    = 0x1000,
	                                .type      = multiboot::tags::MemoryMap::Type::DEFECTIVE,
	                                .reserved  = 0};
	memory_map.other_entries[1]  = {.base_addr = PhysicalAddress(0x12000),
	                                .length    = 0x1000,
	                                .type      = multiboot::tags::MemoryMap::Type::AVAILABLE,
	                                .reserved  = 0};
	bump_allocator.allocate(0); /* Allocate first frame of memory */

	char *bitmap_start    = static_cast<char *>(malloc(32));
	char *bitmap_end      = bitmap_start + 32;
	auto bitmap_allocator = physical_allocators::BitmapAllocator::from(Frame::containing_address(PhysicalAddress(0)),
	                                                                   bump_allocator,
	                                                                   reinterpret_cast<uint64_t *>(bitmap_start),
	                                                                   reinterpret_cast<uint64_t *>(bitmap_end));

	EXPECT_TRUE(bitmap_allocator.get_bit(
			Frame::containing_address(PhysicalAddress(0x0241)))); /* check bump allocated collision */
	EXPECT_TRUE(
			bitmap_allocator.get_bit(Frame::containing_address(PhysicalAddress(0x4245)))); /* check kernel collision */
	EXPECT_TRUE(bitmap_allocator.get_bit(
			Frame::containing_address(PhysicalAddress(0x7813)))); /* check multiboot collision */
	EXPECT_TRUE(bitmap_allocator.get_bit(
			Frame::containing_address(PhysicalAddress(0x10482)))); /* check bad memory collision */
	EXPECT_FALSE(bitmap_allocator.get_bit(Frame::containing_address(PhysicalAddress(0x12284)))); /* check free memory */
}
