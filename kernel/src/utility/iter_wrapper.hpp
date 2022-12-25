
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_UTILITY_ITER_WRAPPER_HPP
#define POPCORN_KERNEL_SRC_UTILITY_ITER_WRAPPER_HPP

#include <cstddef>
#include <compare>

namespace iter {
	template<class T> class iter_wrapper {
	public:
		using value_type      = T;
		using reference       = T&;
		using difference_type = std::size_t;

		iter_wrapper() = delete;
		explicit iter_wrapper(T t) : inner(t) {}

		iter_wrapper& operator++() {
			++this->inner;
			return *this;
		}
		iter_wrapper& operator--() {
			--this->inner;
			return *this;
		}
		bool operator!=(iter_wrapper rhs) { return this->inner != rhs.inner; }
		std::strong_ordering operator<=>(iter_wrapper rhs) { return this->inner <=> rhs.inner; }
		reference operator*() { return this->inner; }
		value_type operator->() { return this->inner; }
		std::size_t operator-(iter_wrapper rhs) { return this->inner - rhs.inner; }

	private:
		T inner;
	};
}

#endif   //POPCORN_KERNEL_SRC_UTILITY_ITER_WRAPPER_HPP
