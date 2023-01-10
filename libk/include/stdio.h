/*
 * Copyright (c) 2023 Oliver Hiorns.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUG_STDIO_H
#define _HUG_STDIO_H

#ifdef HUGOS_TEST
	#include_next <stdio.h>
#else

	#include <stdarg.h>
	#include <stddef.h>
	#include <stdint.h>

	#ifdef __cplusplus
extern "C" {
	#endif

	typedef struct {
		uint32_t fd;
	} FILE;

	typedef uint64_t fpos_t;

	extern FILE *stdin;
	extern FILE *stdout;
	extern FILE *stderr;
	extern FILE *stdserial;

	int remove(const char *filename);
	int rename(const char *oldname, const char *newname);
	FILE *tmpfile(void);
	char *tmpnam(char *str);

	int fclose(FILE *stream);
	int fflush(FILE *stream);
	FILE *fopen(const char *filename, const char *mode);
	FILE *freopen(const char *filename, const char *mode, FILE *stream);
	void setbuf(FILE *stream, char *buffer);
	int setvbuf(FILE *stream, char *buffer, int mode, size_t size);

	int fprintf(FILE *stream, const char *fmt, ...);
	int fscanf(FILE *stream, const char *format, ...);
	int printf(const char *fmt, ...);
	int scanf(const char *format, ...);
	int snprintf(char *str, size_t size, const char *fmt, ...);
	int sprintf(char *str, const char *fmt, ...);
	int asprintf(char **str, const char *fmt, ...);
	int sscanf(const char *s, const char *format, ...);
	int vfprintf(FILE *stream, const char *fmt, va_list args);
	int vfscanf(FILE *stream, const char *format, va_list args);
	int vprintf(const char *fmt, va_list args);
	int vscanf(const char *format, va_list args);
	int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
	int vsprintf(char *str, const char *fmt, va_list args);
	int vsscanf(char *str, const char *fmt, va_list args);

	int fgetc(FILE *stream);
	char *fgets(char *str, int num, FILE *stream);
	void fputc(unsigned char c, FILE *stream);
	void fputs(const char *str, FILE *stream);
	int getc(FILE *stream);
	int getchar(void);
	char *gets(char *str);
	void putc(unsigned char c, FILE *stream);
	void putchar(unsigned char c);
	void puts(const char *str);
	int ungetc(int character, FILE *stream);

	size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
	size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

	int fgetpos(FILE *stream, fpos_t *pos);
	int fseek(FILE *stream, long int offset, int origin);
	int fsetpos(FILE *stream, const fpos_t *pos);
	long int ftell(FILE *stream);
	void rewind(FILE *stream);

	void clearerr(FILE *stream);
	int feof(FILE *stream);
	int ferror(FILE *stream);
	void perror(const char *str);

	void term_clear();
	void handle_esc_code(int code);

	#ifdef __cplusplus
}
	#endif
#endif

#endif
