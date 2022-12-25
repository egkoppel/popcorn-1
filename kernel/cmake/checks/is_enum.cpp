/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

enum E {a};

struct B {
	enum E {b};
};

using enum_t = E;

static_assert(__is_enum(E));
static_assert(__is_enum(enum_t));
static_assert(!__is_enum(B));
static_assert(__is_enum(B::E));
static_assert(!__is_enum(int));
