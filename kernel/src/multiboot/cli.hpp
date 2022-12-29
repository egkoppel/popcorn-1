
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_CLI_HPP
#define HUGOS_CLI_HPP

#include "multiboot.hpp"

namespace multiboot::tags {
	class [[gnu::packed]] Cli : public Tag {
	private:
		char str;

	public:
		Cli(const Cli&) = delete;
		inline const char *args() { return &this->str; }
	};
}   // namespace multiboot::tags

#endif   //HUGOS_CLI_HPP
