
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_FUNCTIONAL_HPP
#define HUGOS_KERNEL_SRC_STL__STL_FUNCTIONAL_HPP

#include <utility>

HUGOS_STL_BEGIN_NAMESPACE
	template<class T = void> struct less {
		constexpr bool operator()(const T& lhs, const T& rhs) const {
			return lhs < rhs;   // assumes that the implementation uses a flat address space
		}
	};

	template<class T = void> struct greater {
		constexpr bool operator()(const T& lhs, const T& rhs) const {
			return lhs > rhs;   // assumes that the implementation uses a flat address space
		}
	};

	template<class T = void> struct less_equal {
		constexpr bool operator()(const T& lhs, const T& rhs) const {
			return lhs <= rhs;   // assumes that the implementation uses a flat address space
		}
	};

	template<class T = void> struct greater_equal {
		constexpr bool operator()(const T& lhs, const T& rhs) const {
			return lhs >= rhs;   // assumes that the implementation uses a flat address space
		}
	};

	template<class T = void> struct equal_to {
		constexpr bool operator()(const T& lhs, const T& rhs) const {
			return lhs == rhs;   // assumes that the implementation uses a flat address space
		}
	};

	template<class T = void> struct not_equal_to {
		constexpr bool operator()(const T& lhs, const T& rhs) const {
			return lhs != rhs;   // assumes that the implementation uses a flat address space
		}
	};

	template<class Fp, class... Args>
	inline constexpr decltype(std::declval<Fp>()(std::declval<Args>()...))
	invoke(Fp && f, Args && ...args) noexcept(noexcept(static_cast<Fp&&>(f)(static_cast<Args&&>(args)...))) {
		return (std::forward<Fp>(f))(std::forward<Args>(args)...);
	}

	template<class T> struct reference_wrapper {
		using type = T;

		template<class U>
		constexpr reference_wrapper(U&& x) requires(!std::is_same_v<reference_wrapper, std::decay_t<U>>)
		{
			T& t      = std::forward<U>(x);
			this->val = &t;
		}
		constexpr reference_wrapper(const reference_wrapper&) noexcept    = default;
		reference_wrapper& operator=(const reference_wrapper& x) noexcept = default;

		constexpr T& get() const noexcept { return *this->val; }
		explicit(false) constexpr operator T &() const noexcept { return this->get(); }

	private:
		T *val;
	};

	template<class T> reference_wrapper(T&) -> reference_wrapper<T>;

HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_FUNCTIONAL_HPP
