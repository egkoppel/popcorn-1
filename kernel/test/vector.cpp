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
#include <stl/vector>
#undef private
#undef protected

TEST(Vector, CreateVectorOfRepeatedElement) {
	hugos_std::vector<int> v = hugos_std::vector<int>(5, 1);
	EXPECT_EQ(v.size(), 5);
	EXPECT_GE(v.capacity(), 5);
	EXPECT_EQ(v[0], 1);
}

TEST(Vector, PushBack) {
	hugos_std::vector<int> v;
	EXPECT_EQ(v.size(), 0);
	v.push_back(7);
	EXPECT_EQ(v.size(), 1);
	EXPECT_GE(v.capacity(), 1);
	v.push_back(23);
	EXPECT_EQ(v.size(), 2);
	EXPECT_GE(v.capacity(), 2);
	EXPECT_EQ(v[0], 7);
	EXPECT_EQ(v[1], 23);
}

TEST(Vector, MoveableType) {
	hugos_std::vector<std::unique_ptr<int>> v;
	EXPECT_EQ(v.size(), 0);
	v.push_back(std::make_unique<int>(3));
	v.push_back(std::make_unique<int>(5));
	EXPECT_EQ(v.size(), 2);
	EXPECT_GE(v.capacity(), 2);
	EXPECT_EQ(*v[0], 3);
	EXPECT_EQ(*v[1], 5);
}

TEST(Vector, TheItemsDontAllGetDuplicated) {
	hugos_std::vector<int> v(5);
	EXPECT_EQ(v.size(), 0);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(5);
	auto cap = v.capacity();
	v.push_back(6);
	v.push_back(7);
	v.push_back(8);
	v.push_back(9);
	ASSERT_GT(v.capacity(), cap);
	EXPECT_EQ(v[0], 2);
	EXPECT_EQ(v[1], 3);
	EXPECT_EQ(v[2], 4);
	EXPECT_EQ(v[3], 5);
	EXPECT_EQ(v[4], 6);
	EXPECT_EQ(v[5], 7);
	EXPECT_EQ(v[6], 8);
	EXPECT_EQ(v[7], 9);
}

TEST(Vector, ProperlyDestroysItems) {
	int cleaned = 0;
	class Item {
	private:
		int& cleaned;

	public:
		explicit Item(int& cleaned) : cleaned(cleaned) {}
		~Item() { this->cleaned++; }
	};

	{
		hugos_std::vector<std::unique_ptr<Item>> v(2);
		EXPECT_GE(v.capacity(), 2);
		v.emplace_back(new Item(cleaned));
		v.emplace_back(new Item(cleaned));
		v.emplace_back(new Item(cleaned));
		v.emplace_back(new Item(cleaned));
		EXPECT_EQ(v.size(), 4);
		EXPECT_GE(v.capacity(), 4);
	}
	// Drop vector by going out of scope - all items should clean up
	EXPECT_EQ(cleaned, 4);
}
