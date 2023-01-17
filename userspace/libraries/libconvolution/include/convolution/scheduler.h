
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_SCHEDULER_H
#define POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_SCHEDULER_H

#include "syscall.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline int64_t convolution_yield() {
	convolution_syscall(0x001);
}

_Noreturn static inline void exit() {
	convolution_syscall(0x002);
}

#ifdef __cplusplus
};
#endif

#endif   // POPCORN_LIBCONVOLUTION_INCLUDE_CONVOLUTION_SCHEDULER_H
