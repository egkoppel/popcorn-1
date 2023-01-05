
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STL__STL_POINTER_ITERATOR_WRAPPER_HPP
#define POPCORN_KERNEL_SRC_STL__STL_POINTER_ITERATOR_WRAPPER_HPP

HUGOS_STL_BEGIN_NAMESPACE
HUGOS_STL_BEGIN_PRIVATE_NAMESPACE
template<class T> class iterator_ptr_wrapper {
public:
	using value_type      = T;
	using reference       = T&;
	using difference_type = std::size_t;

	iterator_ptr_wrapper() = delete;
	explicit iterator_ptr_wrapper(T *t) : inner(t) {}

	iterator_ptr_wrapper& operator++() {
		++this->inner;
		return *this;
	}
	iterator_ptr_wrapper& operator--() {
		--this->inner;
		return *this;
	}
	bool operator!=(iterator_ptr_wrapper rhs) { return this->inner != rhs.inner; }
	std::strong_ordering operator<=>(iterator_ptr_wrapper rhs) { return this->inner <=> rhs.inner; }
	reference operator*() { return *this->inner; }
	value_type *operator->() { return this->inner; }
	std::size_t operator-(iterator_ptr_wrapper rhs) { return this->inner - rhs.inner; }

private:
	T *inner;
};
HUGOS_STL_END_PRIVATE_NAMESPACE
HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL_POINTER_ITERATOR_WRAPPER_HPP
