
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_DEQUE_HPP
#define HUGOS_KERNEL_SRC_STL__STL_DEQUE_HPP

HUGOS_STL_BEGIN_NAMESPACE
template<class T> class deque {
public:
	using value_type      = T;
	using size_type       = size_t;
	using difference_type = ptrdiff_t;
	using reference       = value_type&;
	using const_reference = const value_type&;

private:

public:
	constexpr deque() noexcept;
	constexpr deque(size_type count, const T& value);
	constexpr explicit deque(size_type count);
	constexpr deque(deque&& other) noexcept;
	deque(std::initializer_list<T> init);

	reference front();
	const_reference front() const;
	reference back();
	const_reference back() const;

	[[nodiscard]] bool empty() const noexcept;
	size_type size() const noexcept;

	template<class... Args> reference emplace_front(Args&&...args);
	template<class... Args> reference emplace_back(Args&&...args);

	void push_front(const T& value);
	void push_back(const T& value);
	void push_front(T&& value);
	void push_back(T&& value);

	void pop_front();
	void pop_back();
};
HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_DEQUE_HPP
