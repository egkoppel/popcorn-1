
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_STL__STL_STDEXCEPT_HPP
#define POPCORN_KERNEL_SRC_STL__STL_STDEXCEPT_HPP

#include <exception>
#include <string>

HUGOS_STL_BEGIN_NAMESPACE

class runtime_error : public std::exception {
private:
	struct data_t {
		std::string what_string;
		unsigned long long refcount = 1;
	};

public:
	explicit runtime_error(const std::string& what_arg) :
		data{
				new data_t{.what_string = what_arg, .refcount = 1}
    } {}

	explicit runtime_error(const char *what_arg) :
		data{
				new data_t{.what_string = what_arg, .refcount = 1}
    } {}

	runtime_error(const runtime_error& other) noexcept {
		this->data = other.data;
		++this->data->refcount;
	}

	~runtime_error() override {
		if (--this->data->refcount == 0) { delete this->data; }
	}

	const char *what() const noexcept override { return this->data->what_string.c_str(); }

private:
	data_t *data{};
};

HUGOS_STL_END_NAMESPACE

#endif   //POPCORN_KERNEL_SRC_STL__STL_STDEXCEPT_HPP
