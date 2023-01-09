/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_UTILS_H
#define _HUGOS_UTILS_H

#include <panic.h>

#ifdef __cplusplus

#include <utility>

#define ALIGN_UP(ptr, alignment) ((decltype(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
#define ALIGN_DOWN(ptr, alignment) ((decltype(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
#define ADD_BYTES(ptr, offset) ((decltype(ptr))((uintptr_t)ptr + offset))

#define IDIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

/*template<class T> class Option {
private:
	bool _is_some;
	alignas(alignof(T)) char inner_[sizeof(T)]; // memory for inner object
	T& inner = reinterpret_cast<T&>(inner_);

	Option(T&& inner) : _is_some(true) {
		this->inner = std::move(inner);
	}
	Option() : _is_some(false) {}
public:
	Option(Option<T> const& rhs) : _is_some(rhs._is_some) {
		memcpy(this->inner_, rhs.inner_, sizeof(this->inner_));
	}
	Option<T>& operator =(Option<T> rhs) {
		this->_is_some = rhs._is_some;
		memcpy(this->inner_, rhs.inner_, sizeof(this->inner_));
		return *this;
	}

	static inline Option<T> Some(T&& inner) { return Option(std::move(inner)); }
	static inline Option<T> None() { return Option(); }

	inline bool is_some() const { return this->_is_some; }
	inline bool is_some_and(auto& predicate) const { return this->is_some() && predicate(this->inner); }
	inline bool is_none() const { return !this->is_some(); }
	inline T expect(const char *message) {
		if (this->is_none()) panic(message);
		return this->inner;
	}
	inline T unwrap() { return this->expect("Attempted to unwrap empty optional"); }
	inline T unwrap_or(T val) const { return this->is_some() ? this->inner : val; }
	inline T unwrap_or_else(auto& f) const { return this->is_some() ? this->inner : f(); }
	template<class U> Option<U> map(auto f) const {
		return this->is_some() ? Option<U>::Some(f(this->inner)) : Option<U>::None();
	}
	template<class U> Option<U> and_then(auto f) const { return this->is_some() ? f(this->inner) : Option<U>::None(); }
};

template<class T> [[gnu::always_inline]] extern inline Option<T> Some(T inner) {
	return Option<T>::Some(std::move(inner));
};
template<class T> const auto None = Option<T>::None;*/

#else
#define ALIGN_UP(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
#define ALIGN_DOWN(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
#define ADD_BYTES(ptr, offset) ((typeof(ptr))((uintptr_t)ptr + offset))

#define IDIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#endif

#endif
