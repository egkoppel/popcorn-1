/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

extern void foo(int);
extern void bar(bool) noexcept;

struct C {};
struct NC {
	NC() = default;
	NC(int a) { foo(a); }
	NC(bool b) noexcept { bar(b); }
};

static_assert(__is_nothrow_constructible(C));
static_assert(__is_nothrow_constructible(NC));
static_assert(!__is_nothrow_constructible(NC, int));
static_assert(__is_nothrow_constructible(NC, bool));

