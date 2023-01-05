
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_RANGE_TREE_HPP
#define HUGOS_RANGE_TREE_HPP

#include <memory>
#include <compare>
#include <functional>
#include <utils.h>
#include <tuple>

namespace structures {
	namespace _private {
		template<typename Key, typename Value> struct range_map_node {
			std::unique_ptr<range_map_node> left, right;
			range_map_node *prev, *next;
			Key start, end;
			Value data;
			int8_t balance: 2;

			range_map_node(const Key& start, const Key& end, const Value& data) : left(nullptr), right(nullptr),
			                                                                      prev(nullptr),
			                                                                      next(nullptr), start(start), end(end),
			                                                                      data(std::move(data)), balance(0) {}
		};
	}

	template<typename Key, typename Value, typename Compare = std::less<Key>> class avl_range_map {
	public:
		class iterator {
		public:
			iterator(_private::range_map_node<Key, Value> *) {}
		};

	private:
		using node_t = _private::range_map_node<Key, Value>;
		using direction_t = enum { LEFT, RIGHT };

		node_t *first, *last;
		std::unique_ptr<node_t> root;
		[[gnu::always_inline]] inline std::strong_ordering key_spaceship(const Key& a, const Key& b) {
			return Compare{}(a, b) ? std::strong_ordering::less : Compare{}(b, a)
			                                                      ? std::strong_ordering::greater
			                                                      : std::strong_ordering::equivalent;
		}

		/**
		 * @return (pointer to node pointer if found, pointer to parent node, direction the child was from the parent)
		 */
		std::tuple<std::unique_ptr<node_t> *, node_t *, direction_t> internal_find(const Key& key) {
			auto r = &this->root;
			node_t *prev_r = nullptr;
			auto child_direction = LEFT;
			while (true) {
				if (*r == nullptr) return std::make_tuple(r, prev_r, child_direction);
				auto move_left = this->key_spaceship(key, (*r)->start) < 0; // key < start of test node
				auto move_right = this->key_spaceship(key, (*r)->end) > 0; // key > end of test node
				if (!move_left && !move_right) return std::make_tuple(r, prev_r, child_direction);
				prev_r = r->get();
				if (move_left) {
					r = &(*r)->left;
					child_direction = LEFT;
					continue;
				}
				if (move_right) {
					r = &(*r)->right;
					child_direction = RIGHT;
					continue;
				}
			}
		}

		std::tuple<std::unique_ptr<node_t> *, node_t *, direction_t>
		internal_find(const Key& start, const Key& end) {
			auto r = &this->root;
			node_t *prev_r = nullptr;
			auto child_direction = LEFT;
			while (true) {
				if (*r == nullptr) return std::make_tuple(r, prev_r, child_direction);
				auto move_left = this->key_spaceship(end, (*r)->start) <= 0; // end key <= start of test node
				auto move_right = this->key_spaceship(start, (*r)->end) >= 0; // start key >= end of test node
				if (!move_left && !move_right) return std::make_tuple(r, prev_r, child_direction);
				prev_r = r->get();
				if (move_left) {
					r = &(*r)->left;
					child_direction = LEFT;
					continue;
				}
				if (move_right) {
					r = &(*r)->right;
					child_direction = RIGHT;
					continue;
				}
			}
		}
	public:
		avl_range_map() : first(nullptr), last(nullptr), root(nullptr) {}
		iterator begin();
		iterator end() { return iterator(nullptr); }
		iterator rbegin();
		iterator rend();
		iterator find(const Key& key) {
			auto [ptr, _a, _b] = this->internal_find(key);
			(void)_a;
			(void)_b;
			return ptr ? iterator(ptr) : this->end();
		}
		iterator find(const Key& start, const Key& end) {
			auto [ptr, _a, _b] = this->internal_find(start, end);
			(void)_a;
			(void)_b;
			return ptr ? iterator(ptr) : this->end();
		}
		size_t erase(const Key& key) {
			auto [node, parent, direction] = this->internal_find(key);
			if (!*node) return 0;
			if (!(*node)->left && !(*node)->right) {
				// no children, just drop it
				if (direction == LEFT) parent->left = nullptr;
				else parent->right = nullptr;
			} else if ((*node)->left && !(*node)->right) {
				// only has left child
				if (direction == LEFT) parent->left = std::move((*node)->left);
				else parent->right = std::move((*node)->left);
			} else if (!(*node)->left && (*node)->right) {
				// only has right child
				if (direction == LEFT) parent->left = std::move((*node)->right);
				else parent->right = std::move((*node)->right);
			} else {
				// has both children - pain time
				direction_t dir = LEFT;
				node_t *new_parent_parent_ref = node->get();
				std::unique_ptr<node_t> *new_parent_ref = &(*node)->left;
				while ((*new_parent_ref)->right) {
					dir = RIGHT;
					new_parent_parent_ref = new_parent_ref->get();
					new_parent_ref = &(*new_parent_ref)->right;
				}
				auto new_parent = std::move(dir == RIGHT ? new_parent_parent_ref->right : new_parent_parent_ref->left);
				new_parent->right = std::move((*node)->right);
				new_parent->left = std::move((*node)->left);
				if (direction == LEFT) parent->left = std::move(new_parent);
				else parent->right = std::move(new_parent);
			}
			return 1;
		}
		//bool insert(Key&& start, Key&& end, Value&& value);
		bool insert(const Key& start, const Key& end, const Value& value) {
			auto [node, parent, direction] = this->internal_find(start, end);
			if (*node) return false;
			if (!parent) {
				this->root = std::move(std::make_unique<node_t>(start, end, value));
				return true;
			}
			if (direction == LEFT) {
				parent->left = std::move(std::make_unique<node_t>(start, end, value));
				return true;
			} else {
				parent->right = std::move(std::make_unique<node_t>(start, end, value));
				return true;
			}
		}
	};
}

#endif //HUGOS_RANGE_TREE_HPP
