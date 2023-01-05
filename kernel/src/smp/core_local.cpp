/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core_local.hpp"

#include <arch/amd64/macros.hpp>
#include <log.hpp>
#include <popcorn_prelude.h>
#include <stdlib.h>
#include <utils.h>

constexpr usize FS_BASE_MSR        = 0xC0000100;
constexpr usize GS_BASE_MSR        = 0xC0000101;
constexpr usize GS_KERNEL_BASE_MSR = 0xC0000102;

bool tls_initialised = false;

void create_core_local_data(usize size) {
	auto tls                        = calloc(1, IDIV_ROUND_UP(size, 8) * 8 + 8);
	tls                             = ADD_BYTES(tls, IDIV_ROUND_UP(size, 8) * 8);
	*reinterpret_cast<void **>(tls) = tls;
	wrsmr(FS_BASE_MSR, reinterpret_cast<u64>(tls));
	tls_initialised = true;
}
