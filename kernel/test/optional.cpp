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
#include <stl/optional>
#undef private
#undef protected

TEST(OptionalPtr, BoolsProperly) {
	{
		hugos_std::optional<void *> opt = hugos_std::nullopt;
		EXPECT_FALSE(opt.operator bool());
		EXPECT_FALSE(opt.has_value());
	}

	{
		hugos_std::optional<void *> opt = nullptr;
		EXPECT_FALSE(opt.operator bool());
		EXPECT_FALSE(opt.has_value());
	}

	{
		hugos_std::optional<void *> opt = (void *)0xff834829;
		EXPECT_TRUE(opt.operator bool());
		EXPECT_TRUE(opt.has_value());
	}
}

TEST(Optional, ValueOr) {
	{
		hugos_std::optional<void *> opt = hugos_std::nullopt;
		EXPECT_EQ(opt.value_or([]() { return (void *)0xbadbad; }()), (void *)0xbadbad);
	}

	{
		hugos_std::optional<void *> opt = nullptr;
		EXPECT_EQ(opt.value_or([]() { return (void *)0xbadbad; }()), (void *)0xbadbad);
	}

	{
		hugos_std::optional<void *> opt = (void *)0xff834829;
		EXPECT_EQ(opt.value_or([]() { return (void *)0xbadbad; }()), (void *)0xff834829);
	}
}
