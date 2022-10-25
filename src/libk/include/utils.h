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

template<class T> class Option {
private:
	bool _is_some;
	union {
		T inner;
		struct {} _;
	} data;

	Option(T inner) : _is_some(true), data{.inner = std::move(inner)} {}
	Option() : _is_some(false), data{._ = {}} {}
public:
	static inline Option<T> Some(T inner) { return Option(std::move(inner)); }
	static inline Option<T> None() { return Option(); }

	inline bool is_some() const { return this->_is_some; }
	inline bool is_some_and(auto& predicate) const { return this->is_some() && predicate(this->data.inner); }
	inline bool is_none() const { return !this->is_some(); }
	inline T expect(const char *message) {
		if (this->is_none()) panic(message);
		return this->data.inner;
	}
	inline T unwrap() { return this->expect("Attempted to unwrap empty optional"); }
	inline T unwrap_or(T val) const { return this->is_some() ? this->data.inner : val; }
	inline T unwrap_or_else(auto& f) const { return this->is_some() ? this->data.inner : f(); }
	template<class U> Option<U> map(auto& f) const { return this->is_some() ? Some(f(this->data.inner)) : None(); }
	template<class U> Option<U> and_then(auto& f) const { return this->is_some() ? f(this->data.inner) : None(); }
};

template<class T> const auto Some = Option<T>::Some;
template<class T> const auto None = Option<T>::None;

#else
#define ALIGN_UP(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
#define ALIGN_DOWN(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
#define ADD_BYTES(ptr, offset) ((typeof(ptr))((uintptr_t)ptr + offset))

#define IDIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define UNWRAP(s) ({if (!s.valid) panic("Attempted to unwrap empty optional " #s); s.value;})
#endif

#endif
