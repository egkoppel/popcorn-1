/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_CORE_LOCAL_HPP
#define HUGOS_CORE_LOCAL_HPP

#include <popcorn_prelude.h>

void create_core_local_data(usize size);
extern bool tls_initialised;

#endif   // HUGOS_CORE_LOCAL_HPP
