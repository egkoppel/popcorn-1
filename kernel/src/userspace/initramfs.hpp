/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_INITRAMFS_H
#define _HUGOS_INITRAMFS_H

#include <stdint.h>
#include <stddef.h>

class Initramfs {
private:
	uint64_t data_start;
	uint64_t data_end;
public:
	Initramfs(uint64_t data_start, uint64_t data_end) : data_start(data_start), data_end(data_end) {};
	size_t locate_file(const char *filename, void **data);
};

#endif
