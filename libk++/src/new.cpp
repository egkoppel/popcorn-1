/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <new>
#include <stdlib.h>

void *operator new(size_t size) {
	void *r = malloc(size);
	if (!r) throw std::bad_alloc();
	return r;
}

void *operator new[](size_t size) {
	void *r = malloc(size);
	if (!r) throw std::bad_alloc();
	return r;
}

void operator delete(void *p) noexcept {
	free(p);
}

void operator delete[](void *p) noexcept {
	free(p);
}
