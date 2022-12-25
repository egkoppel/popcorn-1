
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STRUCTURES_STEP_BY_HPP
#define POPCORN_KERNEL_SRC_STRUCTURES_STEP_BY_HPP

#include <iterator>
#include <utility>

namespace iter {
	namespace detail {
		template<class I> concept is_jumpable = requires(I i, typename I::difference_type n) {
													{ i += n } -> std::same_as<I&>;
													{ i + n } -> std::same_as<I>;
												};
	}   // namespace detail

	template<class T, std::size_t N> class step_by_iterator_base {
	public:
		using value_type        = typename T::value_type;
		step_by_iterator_base() = delete;
		value_type operator*() { return *this->iter; }

	protected:
		explicit step_by_iterator_base(T&& iter) : iter{std::forward<T>(iter)} {}

		T iter;
	};

	template<class T, std::size_t N> class step_by_iterator;
	template<class T, std::size_t N>
		requires(!detail::is_jumpable<T>)
	class step_by_iterator<T, N> : public step_by_iterator_base<T, N> {
	public:
		step_by_iterator() = delete;
		explicit step_by_iterator(T&& iter, T&& end) :
			step_by_iterator_base<T, N>{std::forward<T>(iter)},
			end{std::forward<T>(end)} {}

		step_by_iterator& operator++() {
			this->increment_impl(std::make_index_sequence<N>());
			return *this;
		}

		bool operator!=(const step_by_iterator& rhs) { return this->iter != rhs.iter; }

	private:
		T end;

		template<std::size_t... Ns> void increment_impl(std::index_sequence<Ns...>) {
			(((void)Ns, ({
				  if (!(++this->iter != this->end)) return;
			  })),
			 ...);
		}
	};

	template<class T, std::size_t N>
		requires(detail::is_jumpable<T>)
	class step_by_iterator<T, N> : public step_by_iterator_base<T, N> {
	public:
		step_by_iterator() = delete;
		explicit step_by_iterator(T&& iter) : step_by_iterator_base<T, N>{std::forward<T>(iter)} {}

		step_by_iterator& operator++() {
			this->iter += N;
			return *this;
		}

		bool operator==(const step_by_iterator& rhs) { return (rhs.iter - this->iter) < N; }
		bool operator!=(const step_by_iterator& rhs) { return !(*this == rhs); }
	};

	template<class T, std::size_t N> class step_byed {
	private:
		template<class U>
		using iterator_for = typename std::conditional<std::is_const<U>::value,
		                                               typename std::decay<U>::type::const_iterator,
		                                               typename std::decay<U>::type::iterator>::type;

	public:
		step_byed() = delete;
		explicit step_byed(T&& object) : object{std::forward<T>(object)} {}

		auto begin()
			requires(!detail::is_jumpable<iterator_for<T>>)
		{
			using std::begin;
			using std::end;
			return step_by_iterator<iterator_for<T>, N>(begin(object), end(object));
		}
		auto end()
			requires(!detail::is_jumpable<iterator_for<T>>)
		{
			using std::end;
			return step_by_iterator<iterator_for<T>, N>(end(object), end(object));
		}

		auto begin()
			requires(detail::is_jumpable<iterator_for<T>>)
		{
			using std::begin;
			using std::end;
			return step_by_iterator<iterator_for<T>, N>(begin(object));
		}
		auto end()
			requires(detail::is_jumpable<iterator_for<T>>)
		{
			using std::end;
			return step_by_iterator<iterator_for<T>, N>(end(object));
		}

		using iterator       = step_by_iterator<typename std::decay<T>::type::iterator, N>;
		using const_iterator = step_by_iterator<typename std::decay<T>::type::const_iterator, N>;

	private:
		T object;
	};

	template<std::size_t N, class T> auto step_by(T&& obj) { return step_byed<T, N>{std::forward<T>(obj)}; }
}   // namespace iter

#endif   //POPCORN_KERNEL_SRC_STRUCTURES_STEP_BY_HPP
