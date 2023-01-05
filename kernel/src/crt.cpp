/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <cstdio>
#include <exception>
#include <log.hpp>
#include <stdlib.h>
#include <termcolor.h>

extern "C" void *__dso_handle = nullptr;
extern "C" void __cxa_finalize(void *dso) noexcept;

typedef void (*ctor_func)();

extern "C" ctor_func start_ctors;
extern "C" ctor_func end_ctors;
extern "C" ctor_func init_array_start;
extern "C" ctor_func init_array_end;

extern "C" void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr);

extern "C" [[noreturn]] void __cxa_init(uint32_t multiboot_magic, uint32_t multiboot_addr) noexcept try {
	printf("[    ] Running ctors\n\tstart: %lp\n\t  end: %lp\n", &start_ctors, &end_ctors);
	ctor_func *i = &start_ctors;
	while (i < &end_ctors) {
		// printf("Calling constructor at %lp\n", i);
		if (*i != nullptr) (*i)();
		i++;
	}
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Ran ctors\n");

	printf("[    ] Running elements of init array\n\tstart: %lp\n\t  end: %lp\n", &init_array_start, &init_array_end);
	i = &init_array_start;
	while (i < &init_array_end) {
		// printf("Calling constructor at 0x%x\n", i);
		if (*i != nullptr) (*i)();
		i++;
	}
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Done running elements of init array\n");

	kmain(multiboot_magic, multiboot_addr);

	__cxa_finalize(nullptr);

	__builtin_unreachable();
} catch (std::exception& e) {
	LOG(Log::CRITICAL, "Exception unhandled: %s", e.what());
	abort();
	__builtin_unreachable();
} catch (const char *c) {
	LOG(Log::CRITICAL, "Exception unhandled: %s", c);
	abort();
	__builtin_unreachable();
} catch (...) {
	LOG(Log::CRITICAL, "Unknown exception unhandled");
	abort();
	__builtin_unreachable();
}

#define ATEXIT_COUNT 128

typedef struct {
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
} atexit_func_entry_t;

atexit_func_entry_t atexit_funcs[ATEXIT_COUNT] = {{nullptr}};
unsigned int atexit_used                       = 0;

extern "C" int __cxa_atexit(void (*f)(void *), void *objptr, void *dso) noexcept {
	if (atexit_used >= ATEXIT_COUNT) return -1;
	atexit_funcs[atexit_used++] = (atexit_func_entry_t){.destructor_func = f, .obj_ptr = objptr, .dso_handle = dso};
	return 0;
}

extern "C" int __cxa_thread_atexit(void (*f)(void *), void *objptr, void *dso) noexcept {
	return 0;
}

extern "C" void __cxa_finalize(void *dso) noexcept {
	for (int i = ATEXIT_COUNT - 1; i >= 0; --i) {
		if (dso == nullptr || atexit_funcs[i].dso_handle == dso) {
			if (atexit_funcs[i].destructor_func != nullptr) atexit_funcs[i].destructor_func(atexit_funcs[i].obj_ptr);
			atexit_funcs[i].destructor_func = nullptr;
		}
	}
}
