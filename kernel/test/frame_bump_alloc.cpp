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

#define private   public
#define protected public

#include <memory/physical_allocators/monotonic_allocator.hpp>

#undef private
#undef protected

using namespace memory;

struct MemoryMapData {
	multiboot::tags::MemoryMap tag;
	multiboot::tags::MemoryMap::Entry other_entries[2];
} __attribute__((packed));

struct MemoryLayout {
	PhysicalAddress kernel_start;
	PhysicalAddress kernel_end;
	PhysicalAddress multiboot_start;
	PhysicalAddress multiboot_end;
};

class BumpAllocator : public ::testing::Test,
					  public testing::WithParamInterface<MemoryLayout> {
private:
	MemoryMapData memory_map_;

protected:
	physical_allocators::MonotonicAllocator allocator;
	const multiboot::tags::MemoryMap& memory_map = memory_map_.tag;

	BumpAllocator() :
		allocator(PhysicalAddress(0),
	              GetParam().kernel_start,
	              GetParam().kernel_end,
	              GetParam().multiboot_start,
	              GetParam().multiboot_end,
	              &memory_map_.tag) {
		memory_map_.tag.type          = multiboot::TagType::MEMORY_MAP;
		memory_map_.tag.size          = sizeof(memory_map_);
		memory_map_.tag.entry_size    = sizeof(multiboot::tags::MemoryMap::Entry);
		memory_map_.tag.entry_version = 0;
		memory_map_.tag.first_entry =
				multiboot::tags::MemoryMap::Entry{.base_addr = PhysicalAddress(0),
		                                          .length    = 0x10000,
		                                          .type      = multiboot::tags::MemoryMap::Type::AVAILABLE,
		                                          .reserved  = 0};
		memory_map_.other_entries[0] =
				multiboot::tags::MemoryMap::Entry{.base_addr = PhysicalAddress(0x10000),
		                                          .length    = 0x1000,
		                                          .type      = multiboot::tags::MemoryMap::Type::DEFECTIVE,
		                                          .reserved  = 0};
		memory_map_.other_entries[1] =
				multiboot::tags::MemoryMap::Entry{.base_addr = PhysicalAddress(0x12000),
		                                          .length    = 0x1000,
		                                          .type      = multiboot::tags::MemoryMap::Type::AVAILABLE,
		                                          .reserved  = 0};
	}
};

TEST_P(BumpAllocator, ReturnIncreasingFrames) {
	auto last_alloc = *allocator.allocate(0).begin();
	for (int i = 0; i < 32; i++) {
		try {
			auto alloc = *allocator.allocate(0).begin();
			EXPECT_GT(alloc, last_alloc) << "Failed on allocation " << i + 1;
			last_alloc = alloc;
		} catch (std::bad_alloc&) { break; }
	}
}

TEST_P(BumpAllocator, FrameAvoidsKernel) {
	std::vector<Frame> kernel_frames;
	for (auto f = Frame::containing_address(GetParam().kernel_start);
	     f <= Frame::containing_address(GetParam().kernel_end);
	     f++)
		kernel_frames.push_back(f);

	for (int i = 0; i < 32; i++) {
		try {
			auto alloc = allocator.allocate(0);
			for (auto frame : kernel_frames) EXPECT_NE(*alloc.begin(), frame);
		} catch (std::bad_alloc&) { break; }
	}
}

TEST_P(BumpAllocator, FrameAvoidsMultiboot) {
	std::vector<Frame> multiboot_frames;
	for (auto f = Frame::containing_address(GetParam().multiboot_start);
	     f <= Frame::containing_address(GetParam().multiboot_end);
	     f++)
		multiboot_frames.push_back(f);

	for (int i = 0; i < 32; i++) {
		try {
			auto alloc = allocator.allocate(0);
			for (auto frame : multiboot_frames) EXPECT_NE(*alloc.begin(), frame);
		} catch (std::bad_alloc&) { break; }
	}
}

TEST_P(BumpAllocator, FrameAvoidsDefectiveMemory) {
	std::vector<Frame> defective_frames;
	for (auto f = Frame::containing_address(PhysicalAddress(0x10000));
	     f <= Frame::containing_address(PhysicalAddress(0x11fff));
	     f++)
		defective_frames.push_back(f);

	for (int i = 0; i < 32; i++) {
		try {
			auto alloc = allocator.allocate(0);
			for (auto frame : defective_frames) EXPECT_NE(*alloc.begin(), frame);
		} catch (std::bad_alloc&) { break; }
	}
}

INSTANTIATE_TEST_SUITE_P(VaryingMemoryLayouts,
                         BumpAllocator,
                         testing::Values((MemoryLayout){PhysicalAddress{0x1050},
                                                        PhysicalAddress{0x1060},
                                                        PhysicalAddress{0x2035},
                                                        PhysicalAddress{0x2300}}, /* Single page each */
                                         (MemoryLayout){PhysicalAddress{0x1050},
                                                        PhysicalAddress{0x2050},
                                                        PhysicalAddress{0x5500},
                                                        PhysicalAddress{0x6420}}, /* Two pages each */
                                         (MemoryLayout){PhysicalAddress{0x1000},
                                                        PhysicalAddress{0x3fff},
                                                        PhysicalAddress{0x6000},
                                                        PhysicalAddress{0x6fff}}, /* On page boundaries */
                                         (MemoryLayout){PhysicalAddress{0x1030},
                                                        PhysicalAddress{0x2050},
                                                        PhysicalAddress{0x2070},
                                                        PhysicalAddress{0x3080}}, /* Overlapping pages */
                                         (MemoryLayout){PhysicalAddress{0x1030},
                                                        PhysicalAddress{0x5050},
                                                        PhysicalAddress{0x2070},
                                                        PhysicalAddress{0x3080}} /* Overlapping memory */
                                         ));
