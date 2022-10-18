/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>

__attribute__((section(".userspace.text"))) void *memcpy(void *dest, const void *src, size_t num) {
	for (size_t i = 0; i < num; ++i) {
		((char *)dest)[i] = ((char *)src)[i];
	}

	return dest;
}

__attribute__((section(".userspace.text"))) void *memmove(void *dest, const void *src, size_t num) {
	if (dest < src) { // copy left to right
		for (size_t i = 0; i < num; ++i) {
			((char *)dest)[i] = ((char *)src)[i];
		}
	} else { // copy right to left
		for (size_t i = num - 1; i >= 0; --i) {
			((char *)dest)[i] = ((char *)src)[i];
		}
	}

	return dest;
}

__attribute__((section(".userspace.text"))) void *memset(void *ptr, int value, size_t n) {
	for (size_t i = 0; i < n; ++i)
		((unsigned char *)ptr)[i] = (unsigned char)value;
	return ptr;
}

__attribute__((section(".userspace.text"))) size_t strlen(const char *str) {
	size_t len = -1;
	while (str[++len] != '\0');
	return len;
}

__attribute__((section(".userspace.text"))) int memcmp(const void *ptr1, const void *ptr2, size_t count) {
	const char *s1 = ptr1;
	const char *s2 = ptr2;
	while (count-- > 0) {
		if (*s1++ != *s2++)
			return s1[-1] < s2[-1] ? -1 : 1;
	}
	return 0;
}

__attribute__((section(".userspace.text"))) int strcmp(const char *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL) {
		if (s1 < s2) return -1;
		if (s1 > s2) return 1;
		return 0;
	}

	while (1) {
		char c1 = *s1++, c2 = *s2++;

		if (c1 != c2) {
			return c1 - c2;
		}

		if (c1 == '\0') {
			return c1 - c2;
		}
	}
}

__attribute__((section(".userspace.text"))) int strncmp(const char *s1, const char *s2, size_t n) {
	if (s1 == NULL || s2 == NULL) {
		if (s1 < s2) return -1;
		if (s1 > s2) return 1;
		return 0;
	}

	int i = 0;
	while (1) {
		char c1 = *s1++, c2 = *s2++;
		if (i++ >= n) return 0;

		if (c1 != c2) {
			return c1 - c2;
		}

		if (c1 == '\0') {
			return c1 - c2;
		}
	}
}
