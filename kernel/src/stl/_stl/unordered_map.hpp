
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STL__STL_UNORDERED_MAP_HPP
#define POPCORN_KERNEL_SRC_STL__STL_UNORDERED_MAP_HPP

#include <functional>

HUGOS_STL_BEGIN_NAMESPACE
	template<class Key, class T, class Hash = std::hash<Key>> class unordered_map;
HUGOS_STL_END_NAMESPACE

#endif   // POPCORN_KERNEL_SRC_STL__STL_UNORDERED_MAP_HPP
