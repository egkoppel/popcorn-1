
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STRUCTURES_ZIP_HPP
#define POPCORN_KERNEL_SRC_STRUCTURES_ZIP_HPP

#include <iterator>
#include <tuple>

/* Modified version of https://committhis.github.io/2020/10/14/zip-iterator.html */

namespace iter {
	template<class... IterTs> class zip_iterator {
	public:
		using value_type = std::tuple<typename IterTs::value_type...>;

		zip_iterator() = delete;
		explicit zip_iterator(IterTs... iter) : iterators{std::forward<IterTs>(iter)...} {}

		zip_iterator& operator++() {
			std::apply([](auto&&...args) { ((++args), ...); }, iterators);
			return *this;
		}

		bool operator!=(const zip_iterator& rhs) { return !(this->any_equal(rhs)); }
		value_type operator*() {
			return std::apply([](auto&&...args) { return value_type(*args...); }, iterators);
		}

	private:
		std::tuple<IterTs...> iterators;

		template<std::size_t... Ns> bool any_equal_impl(const zip_iterator& rhs, std::index_sequence<Ns...>) {
			return (... || !((std::get<Ns>(this->iterators) != std::get<Ns>(rhs.iterators))));
		}

		bool any_equal(const zip_iterator& rhs) {
			return any_equal_impl(rhs, std::make_index_sequence<std::tuple_size<value_type>::value>());
		}
	};

	template<class... Ts> class zipper {
	private:
		template<class U>
		using iterator_for = typename std::conditional<std::is_const<U>::value,
		                                               typename std::decay<U>::type::const_iterator,
		                                               typename std::decay<U>::type::iterator>::type;

	public:
		using zip_type       = zip_iterator<iterator_for<Ts>...>;
		using iterator       = zip_iterator<typename std::decay<Ts>::type::iterator...>;
		using const_iterator = zip_iterator<typename std::decay<Ts>::type::const_iterator...>;

		template<class... Args> explicit zipper(Args&&...args) : objects{std::forward<Args>(args)...} {}

		zip_type begin() {
			using std::begin;
			return std::apply([](auto&&...args) { return zip_type(begin(args)...); }, objects);
		}
		zip_type end() {
			using std::end;
			return std::apply([](auto&&...args) { return zip_type(end(args)...); }, objects);
		}

	private:
		std::tuple<Ts...> objects;
	};

	template<class... IterTs> auto zip(IterTs&&...iters) {
		return zipper<IterTs...>{std::forward<IterTs>(iters)...};
	}

}   // namespace iter

#endif   // POPCORN_KERNEL_SRC_STRUCTURES_ZIP_HPP
