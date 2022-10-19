/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>
#include <panic.h>
#include <assert.h>

#define ATOI_BUFLEN 65

__attribute__((section(".userspace.rodata"))) static const char itoa_str[] = "0123456789abcdefghijklmnopqrstuvwxyz";

__attribute__((section(".userspace.text"))) char *utoa(uint64_t val, char *str, int base) {
	assert_msg(base >= 2, "base must be between 2 and 36");
	assert_msg(base <= 36, "base must be between 2 and 36");
	__attribute__((section(".userspace.data"))) static char buf[ATOI_BUFLEN] = {0};
	int i = ATOI_BUFLEN - 2; // leave final char as '\0'

	do {
		buf[i] = itoa_str[val % base];

		--i;
		val /= base;
	} while (val != 0 && i >= 0);

	memcpy(str, &buf[i + 1], ATOI_BUFLEN - i - 1);

	return str;
}

__attribute__((section(".userspace.text"))) char *itoa(int64_t val, char *str, int base) {
	bool neg = val < 0;
	if (neg) val *= -1; // abs

	utoa(val, neg ? str + 1 : str, base); // if negative, shift along one to leave room for the '-'

	if (neg) *str = '-';

	return str;
}

__attribute__((section(".userspace.text"))) long long atoll_p(const char **s) {
	long long n = 0;
	while (**s >= '0' && **s <= '9') {
		n = n * 10 + (*(*s)++ - '0');
	}
	return n;
}

__attribute__((section(".userspace.text"))) int atoi_p(const char **s) {
	return (int)atoll_p(s);
}

__attribute__((section(".userspace.text"))) long long atoll(const char *s) {
	long long n = 0;
	while (*s >= '0' && *s <= '9') {
		n = n * 10 + *s++ - '0';
	}
	return n;
}

__attribute__((section(".userspace.text"))) int atoi(const char *s) {
	return (int)atoll(s);
}

void abort(void) {
	panic("abort() called");
}

void exit(int status) {
	panic("exit() called with status code %i", status);
}
