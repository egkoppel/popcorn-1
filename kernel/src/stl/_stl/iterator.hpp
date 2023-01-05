
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_ITERATOR_HPP
#define HUGOS_KERNEL_SRC_STL__STL_ITERATOR_HPP

#include <concepts>

HUGOS_STL_BEGIN_NAMESPACE
	template<class C> constexpr auto begin(C & c)->decltype(c.begin()) {
		return c.begin();
	}
	template<class C> constexpr auto end(C & c)->decltype(c.end()) {
		return c.end();
	}
	template<class T, std::size_t N> constexpr T *begin(T(&arr)[N]) {
		return &arr[0];
	}
	template<class T, std::size_t N> constexpr T *end(T(&arr)[N]) {
		return &arr[N];
	}
	template<class C> constexpr auto rbegin(C & c)->decltype(c.rbegin()) {
		return c.rbegin();
	}
	template<class C> constexpr auto rend(C & c)->decltype(c.rend()) {
		return c.rend();
	}
	template<class C> constexpr auto cbegin(const C& c)->decltype(c.begin()) {
		return c.cbegin();
	}
	template<class C> constexpr auto cend(const C& c)->decltype(c.end()) {
		return c.cend();
	}
	template<class T, std::size_t N> constexpr const T *cbegin(const T(&arr)[N]) {
		return &arr[0];
	}
	template<class T, std::size_t N> constexpr const T *cend(const T(&arr)[N]) {
		return &arr[N];
	}
	template<class C> constexpr auto crbegin(const C& c)->decltype(c.rbegin()) {
		return c.crbegin();
	}
	template<class C> constexpr auto crend(const C& c)->decltype(c.rend()) {
		return c.crend();
	}

	template<class Iter> class reverse_iterator {
	public:
		using iterator_type   = Iter;
		using value_type      = typename Iter::value_type;
		using difference_type = typename Iter::difference_type;
		using pointer         = typename Iter::pointer;
		using reference       = typename Iter::reference;

		constexpr reverse_iterator() = default;
		constexpr explicit reverse_iterator(iterator_type x) : inner(x) {}

		constexpr iterator_type base() const { return this->inner; }

		constexpr reference operator*() const {
			Iter tmp = this->inner;
			return *--tmp;
		}
		constexpr pointer operator->() const { return *this->operator*(); }

		constexpr reverse_iterator& operator++() {
			--this->inner;
			return *this;
		}
		constexpr reverse_iterator& operator--() {
			++this->inner;
			return *this;
		}
		constexpr bool operator!=(reverse_iterator& rhs) { return this->inner != rhs.inner; }

	private:
		iterator_type inner;
	};
HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_ITERATOR_HPP
