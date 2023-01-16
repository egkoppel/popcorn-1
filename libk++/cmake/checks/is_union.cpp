/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

union U {
	int a;
};

struct B {
	union U {
		int b;
	};
};

using union_t = U;

static_assert(__is_union(U));
static_assert(__is_union(union_t));
static_assert(!__is_union(B));
static_assert(__is_union(B::U));
static_assert(!__is_union(int));
