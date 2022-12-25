
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_INCLUDE_MIXIN_HPP
#define HUGOS_KERNEL_INCLUDE_MIXIN_HPP

#include <concepts>

namespace mixin {
	namespace concepts {
		template<class T> concept pre_incrementable = requires(T t) {
														  { ++t } -> std::same_as<T&>;
													  };

		template<class T> concept pre_decrementable = requires(T t) {
														  { --t } -> std::same_as<T&>;
													  };

		template<class T, class U> concept add_assignable = requires(T t, const U& u) {
																{ t.operator+=(u) } -> std::same_as<T&>;
															};
		template<class T, class U> concept subtract_assignable = requires(T t, const U& u) {
																	 { t.operator-=(u) } -> std::same_as<T&>;
																 };
	}   // namespace concepts

	template<class T> /*requires(concepts::pre_incrementable<T> && std::copy_constructible<T>)*/ struct post_increment {
		constexpr T operator++(int) noexcept(
				std::is_nothrow_copy_constructible_v<T>&& noexcept(static_cast<T *>(this)->operator++())) {
			T& self = static_cast<T&>(*this);
			T copy{self};
			++(self);
			return copy;
		}

	};

	template<class T> /*requires(concepts::pre_decrementable<T> && std::copy_constructible<T>)*/ struct post_decrement {
		constexpr T operator--(int) noexcept(
				std::is_nothrow_copy_constructible_v<T>&& noexcept(static_cast<T *>(this)->operator--())) {
			T& self = static_cast<T&>(*this);
			T copy{self};
			--(self);
			return copy;
		}

	};

	template<class T>
	struct unary_post_ops : post_increment<T>,
							post_decrement<T> {
	};

	template<class T, class U = const T&>
	/*requires(concepts::add_assignable<T, U> && std::copy_constructible<T>)*/ struct add {
		constexpr const T operator+(U other) const {
			const T& self = static_cast<const T&>(*this);
			return T{self} += other;
		}

	};

	template<class T, class U = const T&>
	/*requires(concepts::subtract_assignable<T, U> && std::copy_constructible<T>)*/ struct subtract {
		constexpr const T operator-(U other) const {
			const T& self = static_cast<const T&>(*this);
			return T{self} -= other;
		}

	};

	template<class T, class U = T>
	struct binary_ops : add<T, U>,
						subtract<T, U> {
	};
}   // namespace mixin

#endif   //HUGOS_KERNEL_INCLUDE_MIXIN_HPP
