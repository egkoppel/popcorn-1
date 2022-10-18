/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_FSD_HPP
#define HUG_FSD_HPP

#include <stdint.h>

[[noreturn]] int fsd_start(void *online_sem, void *ramfs_data, uint64_t ramfs_size);

#endif //HUG_FSD_HPP
