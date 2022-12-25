/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_PANIC_H
#define _HUGOS_PANIC_H

#include <termcolor.h>
#include <stdio.h>

#ifdef HUGOS_TEST

#include <stdlib.h>

#define panic(msg, ...) \
    { \
        printf("[" TERMCOLOR_RED "ERR!" TERMCOLOR_RESET "]" TERMCOLOR_RED " Kernel panicked at %s:%d:\n\t", __FILE__, __LINE__); \
        printf("%s", msg, ##__VA_ARGS__); \
        printf("\n"); \
        abort(); \
    }

#else

#define panic(msg, ...) \
    { \
        printf("[" TERMCOLOR_RED "ERR!" TERMCOLOR_RESET "]" TERMCOLOR_RED " Kernel panicked at %s:%d:\n\t", __FILE__, __LINE__); \
        printf("%s", msg, ##__VA_ARGS__); \
        printf("\n"); \
        __asm__ volatile("cli; hlt"); \
        while (true) __asm__ volatile(""); \
    }

#endif // HUGOS_TEST

#endif
