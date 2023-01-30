/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>

template<class T, T... Ints> class integer_sequence {
	static constexpr size_t size() noexcept { return sizeof...(Ints); }
};

integer_sequence<int, 0, 1, 2> foo() { return __make_integer_seq<integer_sequence, int, 3>(); }
