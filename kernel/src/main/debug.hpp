/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_DEBUG_H
#define _HUGOS_DEBUG_H

#include <popcorn_prelude.h>
#include <stdint.h>

void trace_stack_trace(unsigned int MaxFrames, u64 rbp) noexcept;

#endif
