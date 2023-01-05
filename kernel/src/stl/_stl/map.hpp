
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_MAP_HPP
#define HUGOS_KERNEL_SRC_STL__STL_MAP_HPP

#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

HUGOS_STL_BEGIN_NAMESPACE
template<class Key, class T, class Compare = std::less<Key>> class map {
public:
	using key_type        = Key;
	using mapped_type     = T;
	using value_type      = std::pair<const Key, T>;
	using size_type       = size_t;
	using difference_type = ptrdiff_t;
	using key_compare     = Compare;
	using reference       = value_type&;
	using const_reference = const value_type&;

private:
	class map_node_;

public:
	class map_iterator_ {
	private:
		std::unique_ptr<map_node_>& current;

	public:
		explicit map_iterator_(std::unique_ptr<map_node_>& current) : current(current) {}
		value_type& operator*() { return current->data; }
		value_type *operator->() { return &current->data; }
		bool operator<(map_iterator_ rhs);
		bool operator!=(map_iterator_ rhs);
		bool operator==(map_iterator_ rhs);
	};

	using iterator       = map_iterator_;
	using const_iterator = const iterator;

private:
	class map_node_ {
		friend class map_iterator_;

	private:
		value_type data;
		std::unique_ptr<map_node_> l, r;

		map_node_(value_type&& data) : data(move(data)), l(nullptr), r(nullptr) {}

	public:
		map_node_(const map_node_& other) :
			data{other.data},
			l{other.l ? new map_node_{*other.l} : nullptr},
			r{other.r ? new map_node_{*other.r} : nullptr} {}

		std::unique_ptr<map_node_> new_node(value_type&& data) { return std::unique_ptr(new map_node_(move(data))); }
		void add_l_child(std::unique_ptr<map_node_> c) { this->l = move(c); }
		void add_r_child(std::unique_ptr<map_node_> c) { this->r = move(c); }
		map_node_& left() const { return *this->l; }
		map_node_& right() const { return *this->r; }
	};

	std::unique_ptr<map_node_> head = nullptr;
	const Compare comparator        = Compare{};

public:
	map() = default;
	explicit map(const Compare& comp) : comparator(comp){};
	map(const map& other)
		requires(std::is_copy_constructible_v<Key> && std::is_copy_constructible_v<T>
	             && std::is_copy_constructible_v<Compare>)
		: head{other.head ? new map_node_{*other.head} : nullptr}, comparator{other.comparator} {}
	map(map&& other);
	map(std::initializer_list<value_type> init, const Compare& comp = Compare());

	iterator find(const Key& key);
	const_iterator find(const Key& key) const;

	std::pair<iterator, bool> insert(const value_type& value) { __builtin_unreachable(); }
	std::pair<iterator, bool> insert(value_type&& value) { __builtin_unreachable(); }
	template<class P>
	std::pair<iterator, bool> insert(P&& value)
		requires(std::is_constructible<value_type, P &&>::value);
	iterator insert(const_iterator pos, const value_type& value);
	void insert(std::initializer_list<value_type> ilist);
	template<class... Args> std::pair<iterator, bool> emplace(Args&&...args) {
		return this->insert(std::forward<Args>(args)...);
	}

	iterator begin() noexcept;
	iterator end() noexcept;

	size_type size() const noexcept;
	[[nodiscard]] bool empty() const noexcept { return this->begin() == this->end(); }

	size_type erase(const Key& key);
};

template<class Key, class T, class Comp = std::less<Key>>
map(std::initializer_list<std::pair<Key, T>>, Comp = Comp()) -> map<Key, T, Comp>;

template<class Key, class T>

map(std::initializer_list<std::pair<Key, T>>) -> map<Key, T, std::less<Key>>;
HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_MAP_HPP
