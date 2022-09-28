/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_STACK_H
#define _HUGOS_STACK_H

#include <stdint.h>
#include <stdio.h>
#include "allocator.h"

extern "C" struct Stack {
	public:
	uint64_t top;
	uint64_t bottom;

	Stack(uint64_t size, bool user_access = false);
	Stack(uint64_t top, uint64_t bottom): top(top), bottom(bottom) {}
	~Stack();
};

#endif