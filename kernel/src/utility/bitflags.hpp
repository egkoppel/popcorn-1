/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_UTILITY_BITFLAGS_HPP
#define POPCORN_KERNEL_SRC_UTILITY_BITFLAGS_HPP

#include <cstddef>
#include <tuple>



template<class Underlying> class InvertedFlag;

template<class Underlying> class Flag {
public:
	explicit(false) Flag(Underlying val) : value(val)
	InvertedFlag<Underlying> operator~();

private:
	Underlying value;
};

template<class Underlying> class InvertedFlag {
	friend class Flag<Underlying>;

private:
	explicit InvertedFlag(Flag<Underlying> flag) : flag(flag) {}
	Flag<Underlying> flag;
};

template<class Underlying> InvertedFlag<Underlying> Flag<Underlying>::operator~() {
	return InvertedFlag<Underlying>(this->value);
}

template<class E> class Bitflags {
public:
	using underlying_type = std::underlying_type_t<E>;

	template<class... Args> Bitflags& operator=(std::tuple<Args...>) {

	}

	using enum E;
private:
	underlying_type value;
};

#endif   //POPCORN_KERNEL_SRC_UTILITY_BITFLAGS_HPP
