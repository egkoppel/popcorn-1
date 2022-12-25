
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_UTILITY_REVERSE_HPP
#define POPCORN_KERNEL_SRC_UTILITY_REVERSE_HPP

#include <iterator>

namespace iter {
	/* from https://stackoverflow.com/a/28139075/7406863 */

	template<typename T> struct reversion_wrapper {
		T& iterable;
	};

	template<typename T> auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

	template<typename T> auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

	template<typename T> reversion_wrapper<T> reverse(T&& iterable) { return {iterable}; }
}   // namespace iter

#endif   //POPCORN_KERNEL_SRC_UTILITY_REVERSE_HPP
