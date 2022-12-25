
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_CONCEPTS_HPP
#define HUGOS_KERNEL_SRC_STL__STL_CONCEPTS_HPP

#include <type_traits>

HUGOS_STL_BEGIN_NAMESPACE
namespace detail {
	template<class T, class U> concept SameHelper = std::is_same<T, U>::value;
}
template<class T, class U> concept same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

template<class T> concept copy_constructible = std::is_copy_constructible_v<T>;
template<class T> concept movable = std::is_move_constructible_v<T>;

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_CONCEPTS_HPP
