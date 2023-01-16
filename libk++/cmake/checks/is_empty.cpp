/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

struct A {};
struct B {
	int i;
};
class C {
	static int i;
};
union D {};

static_assert(__is_empty(A));
static_assert(!__is_empty(B));
static_assert(__is_empty(C));
static_assert(!__is_empty(D));
static_assert(!__is_empty(int));
