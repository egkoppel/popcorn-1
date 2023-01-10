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

#include <structures/range_tree.hpp>

#undef private
#undef protected

using namespace structures;
using namespace structures::_private;

class RangeTree : public ::testing::Test {
protected:
	avl_range_map<int, int> tree;

	void print_node(avl_range_map<int, int>::node_t *node, size_t tabs = 0) {
		if (!node) return;
		print_node(node->right.get(), tabs + 1);
		std::cout << std::string(tabs, '\t') << node->data << std::endl;
		print_node(node->left.get(), tabs + 1);
	}

	void print_tree() {
		this->print_node(tree.root.get());
		std::cout << std::endl;
	}
};

TEST_F(RangeTree, BasicTest) {
	EXPECT_TRUE(tree.insert(5, 6, 9));
	EXPECT_TRUE(tree.insert(9, 10, 54));
	EXPECT_TRUE(tree.insert(6, 8, 23));
	EXPECT_TRUE(tree.insert(1, 4, 848));
	EXPECT_TRUE(tree.insert(11, 14, 12));
	EXPECT_TRUE(tree.insert(8, 9, 7));
	print_tree();
	EXPECT_EQ(tree.erase(3), 1);
	print_tree();
	EXPECT_EQ(tree.erase(9), 1);
	print_tree();
}