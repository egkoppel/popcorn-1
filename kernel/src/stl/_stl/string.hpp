
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_STRING_HPP
#define HUGOS_KERNEL_SRC_STL__STL_STRING_HPP

#include <initializer_list>
#include <string.h>

HUGOS_STL_BEGIN_NAMESPACE
template<class CharT> class basic_string {
private:
	CharT *_start;
	CharT *_end;

public:
	basic_string() : _start(nullptr), _end(nullptr) {}
	constexpr basic_string(size_t count, CharT ch) {
		this->_start = new CharT[(count + 1) * sizeof(CharT)];
		this->_end   = this->_start + (count + 1);
		for (size_t i = 0; i < count; ++i) this->_start[i] = ch;
		this->_start[count] = '\0';
	}
	constexpr basic_string(const basic_string& other) {
		this->_start = new CharT[(other.size() + 1) * sizeof(CharT)];
		this->_end   = this->_start + other.size() + 1;
		for (size_t i = 0; i < other.size(); ++i) this->_start[i] = other._start[i];
		this->_start[other.size()] = '\0';
	}
	constexpr basic_string(const basic_string& other, size_t pos, size_t count) {
		this->_start = new CharT[(count + 1) * sizeof(CharT)];
		this->_end   = this->_start + count + 1;
		for (size_t i = 0; i < count; ++i) this->_start[i] = other._start[pos + i];
		this->_start[count] = '\0';
	}
	constexpr basic_string(const CharT *s) {
		auto count   = strlen(s);
		this->_start = new CharT[(count + 1) * sizeof(CharT)];
		this->_end   = this->_start + count + 1;
		for (size_t i = 0; i < count; ++i) this->_start[i] = s[i];
		this->_start[count] = '\0';
	}
	template<class InputIt> constexpr basic_string(InputIt first, InputIt last);
	constexpr basic_string(basic_string&& other) noexcept {
		this->_start = other._start;
		this->_end   = other._end;
		other._start = nullptr;
		other._end   = nullptr;
	}
	constexpr basic_string(std::initializer_list<CharT> ilist) : basic_string(ilist.begin(), ilist.end()) {}

	~basic_string() { delete[] (this->_start); }

	constexpr basic_string& operator=(const basic_string& str);
	constexpr basic_string& operator=(basic_string&& str) noexcept;
	constexpr basic_string& operator=(const CharT *str);
	constexpr basic_string& operator=(decltype(nullptr)) = delete;

	constexpr const CharT *data() const noexcept { return this->_start; }
	constexpr const CharT *c_str() const noexcept { return this->_start; }

	constexpr size_t size() const noexcept { return this->_end - this->_start - 1; }
	constexpr size_t length() const noexcept { return this->_end - this->_start - 1; }
};

using string = basic_string<char>;
HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_STRING_HPP
