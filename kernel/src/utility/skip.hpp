
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STRUCTURES_SKIP_HPP
#define POPCORN_KERNEL_SRC_STRUCTURES_SKIP_HPP

#include <iterator>
#include <utility>

namespace iter {
	template<class T, std::size_t N> class skipped {
	private:
		template<class U>
		using iterator_for = typename std::conditional<std::is_const<U>::value,
		                                               typename std::decay<U>::type::const_iterator,
		                                               typename std::decay<U>::type::iterator>::type;

	public:
		skipped() = delete;
		explicit skipped(T&& object) : object{std::forward<T>(object)} {}

		auto begin() -> iterator_for<T> { return this->begin_impl(std::make_index_sequence<N>()); }
		auto end() -> iterator_for<T> {
			using std::end;
			return end(this->object);
		}

		using iterator       = typename std::decay<T>::type::iterator;
		using const_iterator = typename std::decay<T>::type::const_iterator;

		T object;

	private:
		template<std::size_t... Ns> auto begin_impl(std::index_sequence<Ns...>) -> iterator_for<T> {
			using std::begin;
			auto ret = begin(this->object);
			return (((void)Ns, ++ret), ...);
		}
	};

	template<std::size_t N, class T> auto skip(T&& obj) { return skipped<T, N>{std::forward<T>(obj)}; }
}   // namespace iter

#endif   //POPCORN_KERNEL_SRC_STRUCTURES_SKIP_HPP
