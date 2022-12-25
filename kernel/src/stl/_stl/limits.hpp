
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_LIMITS_HPP
#define HUGOS_KERNEL_SRC_STL__STL_LIMITS_HPP

#include <limits.h>

HUGOS_STL_BEGIN_NAMESPACE

template<class T> struct numeric_limits;

template<> struct numeric_limits<char> {
	static constexpr char min() noexcept { return CHAR_MIN; }
	static constexpr char max() noexcept { return CHAR_MAX; }
};

template<> struct numeric_limits<int> {
	static constexpr int min() noexcept { return INT_MIN; }
	static constexpr int max() noexcept { return INT_MAX; }
};

template<> struct numeric_limits<unsigned int> {
	static constexpr unsigned int min() noexcept { return 0; }
	static constexpr unsigned int max() noexcept { return UINT_MAX; }
};

template<> struct numeric_limits<long int> {
	static constexpr long int min() noexcept { return LONG_MIN; }
	static constexpr long int max() noexcept { return LONG_MAX; }
};

template<> struct numeric_limits<unsigned long int> {
	static constexpr unsigned long int min() noexcept { return 0; }
	static constexpr unsigned long int max() noexcept { return ULONG_MAX; }
};

template<> struct numeric_limits<long long int> {
	static constexpr long int min() noexcept { return LONG_LONG_MIN; }
	static constexpr long int max() noexcept { return LONG_LONG_MAX; }
};

template<> struct numeric_limits<unsigned long long int> {
	static constexpr unsigned long long int min() noexcept { return 0; }
	static constexpr unsigned long long int max() noexcept { return ULONG_LONG_MAX; }
};

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_LIMITS_HPP
