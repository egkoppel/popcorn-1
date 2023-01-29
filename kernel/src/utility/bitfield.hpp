
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_UTILITY_BITFIELD_HPP
#define POPCORN_KERNEL_SRC_UTILITY_BITFIELD_HPP

#include <cstddef>
#include <functional>
#include <mixin.hpp>

template<class T, std::size_t shift, std::size_t mask, class U = T>
class bitfield_member : public mixin::unary_post_ops<bitfield_member<T, shift, mask, U>> {
public:
	bitfield_member(T& backing) : backing(backing) {}
	explicit(false) operator U() { return (this->backing.get() >> shift) & mask; }
	bitfield_member& operator=(U val) {
		this->backing.get() &= ~(mask << shift);
		this->backing.get() |= (val & mask) << shift;
		return *this;
	}
	bitfield_member& operator&=(U val) {
		this->backing.get() &= (val & mask) << shift;
		return *this;
	}
	bitfield_member& operator|=(U val) {
		this->backing.get() |= (val & mask) << shift;
		return *this;
	}
	bitfield_member& operator^=(U val) {
		this->backing.get() ^= (val & mask) << shift;
		return *this;
	}

private:
	std::reference_wrapper<T> backing;
};

#endif   // POPCORN_KERNEL_SRC_UTILITY_BITFIELD_HPP
