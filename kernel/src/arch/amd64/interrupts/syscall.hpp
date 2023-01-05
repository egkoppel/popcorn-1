
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_SYSCALL_HPP
#define HUGOS_SYSCALL_HPP

namespace arch::amd64 {
	void syscall_register_init() noexcept;
}

#endif //HUGOS_SYSCALL_HPP
