/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class C {int a;};

struct B {
	class C {int b;};
};

union U {
	C a;
};

using class_t = C;

static_assert(__is_class(C));
static_assert(__is_class(class_t));
static_assert(__is_class(B));
static_assert(__is_class(B::C));
static_assert(!__is_class(U));
static_assert(!__is_class(int));
