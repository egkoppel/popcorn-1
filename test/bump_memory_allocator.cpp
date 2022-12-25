/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define HUGOS_TEST_MODULE

#include "memory/frame_bump_alloc.hpp"
#include "gtest/gtest.h"

class BumpAllocatorTest : public ::testing::Test {
protected:
	FrameBumpAllocator allocator;

	BumpAllocatorTest(): allocator({0x1000}, {0x2000}, {0x4000}, {0x5000}, nullptr) {}
};

TEST_F(BumpAllocatorTest, TestAllocationIncreasing) {
	auto frame1 = allocator.allocate();
}
