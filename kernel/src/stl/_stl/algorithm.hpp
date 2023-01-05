
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_ALGORITHM_HPP
#define HUGOS_KERNEL_SRC_STL__STL_ALGORITHM_HPP

#include <type_traits>
#include <utility>

HUGOS_STL_BEGIN_NAMESPACE

template<class T> void swap(T& a, T& b) noexcept(noexcept(T(std::move(b)))) {
	T tmp(std::move(b));
	b = std::move(a);
	a = std::move(tmp);
}

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_ALGORITHM_HPP
