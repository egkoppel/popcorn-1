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
#include <fmt/core.h>

#define private public
#define protected public

#include <structures/ring_buffer.hpp>

#undef private
#undef protected

TEST(RingBuffer, PushThenPopReturnsSame) {
	structures::ring_buffer<int, 8> buf;
	ASSERT_TRUE(buf.push(6));
	EXPECT_EQ(buf.pop(), 6);
}

TEST(RingBuffer, SizeCorrect) {
	structures::ring_buffer<int, 8> buf;
	EXPECT_EQ(buf.size(), 0);
	ASSERT_TRUE(buf.push(6));
	EXPECT_EQ(buf.size(), 1);
	ASSERT_TRUE(buf.push(8));
	EXPECT_EQ(buf.size(), 2);
	ASSERT_TRUE(buf.push(2));
	EXPECT_EQ(buf.size(), 3);
	buf.pop();
	EXPECT_EQ(buf.size(), 2);
	buf.pop();
	EXPECT_EQ(buf.size(), 1);
	buf.pop();
	EXPECT_EQ(buf.size(), 0);
}

TEST(RingBuffer, FailsAfterSizeLimit) {
	structures::ring_buffer<int, 2> buf;
	EXPECT_TRUE(buf.push(6));
	EXPECT_TRUE(buf.push(8));
	EXPECT_EQ(buf.size(), 2);
	EXPECT_FALSE(buf.push(2));
}
