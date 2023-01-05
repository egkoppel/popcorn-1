/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_WCHAR_H
#define _HUGOS_WCHAR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int mbstate_t;
typedef size_t off_t;
typedef int wint_t;
struct tm {
	unsigned long long t;
};

int fwide(FILE* stream, int mode);
int fwprintf(FILE *stream, const wchar_t *fmt, ...);
int fwscanf(FILE *stream, const wchar_t *format, ...);
int wprintf(const wchar_t *fmt, ...);
int wscanf(const wchar_t *format, ...);
int swprintf(char *str, const wchar_t *fmt, ...);
int swscanf(const wchar_t *s, const wchar_t *format, ...);
int vfwprintf(FILE *stream, const wchar_t *fmt, va_list args);
int vfwscanf(FILE *stream, const wchar_t *format, va_list args);
int vwprintf(const wchar_t *fmt, va_list args);
int vwscanf(const wchar_t *format, va_list args);
int vswprintf(wchar_t *str, const wchar_t *fmt, va_list args);
int vswscanf(wchar_t *str, const wchar_t *fmt, va_list args);

int fgetwc(FILE *stream);
wchar_t* fgetws(wchar_t *str, int num, FILE *stream);
void fputwc(wchar_t c, FILE *stream);
void fputws(const wchar_t* str, FILE *stream);
int getwc(FILE *stream);
int getwchar(void);
wchar_t* getws(wchar_t *str);
void putwc(wchar_t c, FILE *stream);
void putwchar(wchar_t c);
void putws(const wchar_t* str);
int ungetwc(wchar_t character, FILE *stream);

double wcstod(const wchar_t* str, wchar_t** endptr);
float wcstof(const wchar_t* str, wchar_t** endptr);
long wcstol(const wchar_t* str, wchar_t** endptr);
long long wcstoll(const wchar_t* str, wchar_t** endptr);
unsigned long wcstoul(const wchar_t* str, wchar_t** endptr);
unsigned long long wcstoull(const wchar_t* str, wchar_t** endptr);
long double wcstold(const wchar_t* str, wchar_t** endptr);

int wctob (wint_t wc);

wchar_t* wcscat (wchar_t* destination, const wchar_t* source);
wchar_t* wcschr (const wchar_t* ws, wchar_t wc);
int wcscmp (const wchar_t* wcs1, const wchar_t* wcs2);
int wcscoll (const wchar_t* wcs1, const wchar_t* wcs2);
wchar_t* wcscpy (wchar_t* destination, const wchar_t* source);
size_t wcscspn (const wchar_t* wcs1, const wchar_t* wcs2);
size_t wcslen (const wchar_t* wcs);
wchar_t* wcsncat (wchar_t* destination, const wchar_t* source, size_t num);
int wcsncmp (const wchar_t* wcs1, const wchar_t* wcs2, size_t num);
wchar_t* wcsncpy (wchar_t* destination, const wchar_t* source, size_t num);
wchar_t* wcspbrk (const wchar_t* wcs1, const wchar_t* wcs2);
wchar_t* wcsrchr (const wchar_t* ws, wchar_t wc);
size_t wcsspn (const wchar_t* wcs1, const wchar_t* wcs2);
wchar_t* wcsstr (const wchar_t* wcs1, const wchar_t* wcs2);
wchar_t* wcstok (wchar_t* wcs, const wchar_t* delimiters, wchar_t** p);
size_t wcsxfrm (wchar_t* destination, const wchar_t* source, size_t num);
wchar_t* wmemchr (const wchar_t* ptr, wchar_t wc, size_t num);
int wmemcmp (const wchar_t* ptr1, const wchar_t* ptr2, size_t num);
wchar_t* wmemcpy (wchar_t* destination, const wchar_t* source, size_t num);
wchar_t* wmemmove (wchar_t* destination, const wchar_t* source, size_t num);
wchar_t* wmemset (wchar_t* ptr, wchar_t wc, size_t num);

size_t wcsftime (wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr);

#ifdef __cplusplus
}
#endif

#endif