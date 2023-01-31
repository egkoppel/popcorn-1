
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

#include <cstring>
#include <initializer_list>
#include <utility>
#include <vector>

HUGOS_STL_BEGIN_NAMESPACE
	template<class CharT> class basic_string : private std::vector<CharT> {
	public:
		constexpr basic_string() : std::vector<CharT>() {}
		constexpr basic_string(size_t count, CharT ch) : std::vector<CharT>(count + 1) {
			for (size_t i = 0; i < count; ++i) this->push_back(ch);
			this->operator[](count) = '\0';
		}
		constexpr basic_string(const basic_string& other) : std::vector<CharT>(other) {}

		constexpr basic_string(const basic_string& other, size_t pos, size_t count) : std::vector<CharT>(count + 1) {
			this->item_count_ = count + 1;
			std::memcpy(const_cast<char *>(this->data()), other.data() + pos, count);
			this->operator[](count) = '\0';
		}
		constexpr explicit basic_string(const CharT *s) : basic_string(s, std::strlen(s)) {}

		constexpr basic_string(const CharT *s, usize count) : std::vector<CharT>(count + 1) {
			this->item_count_ = count + 1;
			std::memcpy(const_cast<char *>(this->data()), s, count);
			this->operator[](count) = '\0';
		}

		template<class InputIt> constexpr basic_string(InputIt first, InputIt last);
		constexpr basic_string(basic_string&& other) noexcept : std::vector<CharT>(std::move(other)) {}
		constexpr basic_string(std::initializer_list<CharT> ilist) : basic_string(ilist.begin(), ilist.end()) {}

		constexpr ~basic_string() = default;

		constexpr basic_string& operator=(const basic_string& str);
		constexpr basic_string& operator=(basic_string&& str) noexcept;
		constexpr basic_string& operator=(const CharT *str);
		constexpr basic_string& operator=(decltype(nullptr)) = delete;

		constexpr const CharT *data() const noexcept { return &*this->begin(); }
		constexpr const CharT *c_str() const noexcept { return &*this->begin(); }

		constexpr size_t size() const noexcept { return this->std::vector<CharT>::size() - 1; }
		constexpr size_t length() const noexcept { return this->size(); }

		template<class CharT_>
		constexpr friend std::basic_string<CharT_> operator+(const std::basic_string<CharT_>& lhs,
		                                                     const std::basic_string<CharT_>& rhs) {
			return std::basic_string<CharT_>(lhs, rhs, op_t{});
		}

		template<class CharT_>
		constexpr friend std::basic_string<CharT_> operator+(const std::basic_string<CharT_>& lhs, CharT_ rhs) {
			return std::basic_string<CharT_>(lhs, rhs, op_t{});
		}

		template<class CharT_>
		constexpr friend std::basic_string<CharT_> operator+(const std::basic_string<CharT_>& lhs, const CharT_ *rhs) {
			return std::basic_string<CharT_>(lhs, rhs, op_t{});
		}

	private:
		struct op_t {};

		constexpr explicit basic_string(const basic_string& first, const basic_string& second, op_t)
			: std::vector<CharT>(first.size() + second.size() + 1) {
			this->item_count_ = first.size() + second.size() + 1;
			std::memcpy(this->data(), first.data(), first.size());
			std::memcpy(this->data() + first.size(), second.data(), second.size());
			this->operator[](first.size() + second.size()) = '\0';
		}
		constexpr explicit basic_string(const basic_string& first, CharT second, op_t)
			: std::vector<CharT>(first.size() + 2) {
			this->item_count_ = first.size() + 2;
			std::memcpy(const_cast<CharT *>(this->data()), first.data(), first.size() * sizeof(CharT));
			this->operator[](first.size())     = second;
			this->operator[](first.size() + 1) = '\0';
		}
		constexpr explicit basic_string(const basic_string& first, const CharT *second, op_t)
			: std::vector<CharT>(first.size() + std::strlen(second) + 1) {
			this->item_count_ = first.size() + std::strlen(second) + 1;
			std::memcpy(const_cast<CharT *>(this->data()), first.data(), first.size() * sizeof(CharT));
			std::strcpy(const_cast<CharT *>(this->data()) + first.size(), second);
		}
	};

	using string = basic_string<char>;

	inline namespace literals {
		inline namespace string_literals {
			constexpr std::string operator""s(const char *str, std::size_t len) {
				return {str, len};
			}
		}   // namespace string_literals
	}       // namespace literals

HUGOS_STL_END_NAMESPACE

#endif   // HUGOS_KERNEL_SRC_STL__STL_STRING_HPP
