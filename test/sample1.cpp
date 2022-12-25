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

int Factorial(int n) {
	if (n == 0) return 1;
	return n*Factorial(n-1);
}

TEST(FactorialTest, HandlesZeroInput) {
	EXPECT_EQ(Factorial(0), 1);
}

// Tests factorial of positive numbers.
TEST(FactorialTest, HandlesPositiveInput) {
	EXPECT_EQ(Factorial(1), 1);
	EXPECT_EQ(Factorial(2), 2);
	EXPECT_EQ(Factorial(3), 6);
	EXPECT_EQ(Factorial(8), 40320);
}