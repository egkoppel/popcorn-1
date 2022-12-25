/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define private   public
#define protected public
#include "memory/types.hpp"
#undef private
#undef protected

#include <gtest/gtest.h>

using namespace memory;

TEST(FrameVector, CountsInternalSize) {
	FrameVector single_frame = {Frame::containing_address(PhysicalAddress(0))};
	EXPECT_TRUE(single_frame.is_one());

	FrameVector single_range = {
			{Frame::containing_address(PhysicalAddress(0)), Frame::containing_address(PhysicalAddress(0xfffffff))}
    };
	EXPECT_TRUE(single_range.is_one());

	FrameVector multiple_range = {Frame::containing_address(PhysicalAddress(0)),
	                              Frame::containing_address(PhysicalAddress(0xfffffff))};
	EXPECT_FALSE(multiple_range.is_one());
}

TEST(FrameVector, IteratorBeginIsFirstElement) {
	FrameVector single_frame = {Frame::containing_address(PhysicalAddress(0))};
	EXPECT_EQ(*single_frame.begin(), Frame::containing_address(PhysicalAddress(0)));

	FrameVector single_range = {
			{Frame::containing_address(PhysicalAddress(0)), Frame::containing_address(PhysicalAddress(0xfffffff))}
    };
	EXPECT_EQ(*single_range.begin(), Frame::containing_address(PhysicalAddress(0)));

	FrameVector multiple_range = {Frame::containing_address(PhysicalAddress(0)),
	                              Frame::containing_address(PhysicalAddress(0xfffffff))};
	EXPECT_EQ(*multiple_range.begin(), Frame::containing_address(PhysicalAddress(0)));
}

TEST(FrameVector, IteratorBeginAndEndDifferent) {
	FrameVector single_frame = {Frame::containing_address(PhysicalAddress(0))};
	EXPECT_NE(single_frame.begin(), single_frame.end());

	FrameVector single_range = {
			{Frame::containing_address(PhysicalAddress(0)), Frame::containing_address(PhysicalAddress(0xfffffff))}
    };
	EXPECT_NE(single_range.begin(), single_range.end());

	FrameVector multiple_range = {Frame::containing_address(PhysicalAddress(0)),
	                              Frame::containing_address(PhysicalAddress(0xfffffff))};
	EXPECT_NE(multiple_range.begin(), multiple_range.end());

	FrameVector empty = {};
	EXPECT_EQ(empty.begin(), empty.end());
}

TEST(FrameVector, IteratingCoversEntireRange) {
	FrameVector vec = {
			{Frame::from_address(PhysicalAddress(0)), Frame::from_address(PhysicalAddress(Frame::size * 2))},
			Frame::from_address(PhysicalAddress(Frame::size * 5)),
			Frame::from_address(PhysicalAddress(Frame::size * 6))
    };

	auto iter = vec.begin();
	EXPECT_EQ(*iter, Frame::from_address(PhysicalAddress(0)));
	EXPECT_EQ(*++iter, Frame::from_address(PhysicalAddress(Frame::size)));
	EXPECT_EQ(*++iter, Frame::from_address(PhysicalAddress(Frame::size * 5)));
	EXPECT_EQ(*++iter, Frame::from_address(PhysicalAddress(Frame::size * 6)));
	EXPECT_EQ(++iter, vec.end());
}
