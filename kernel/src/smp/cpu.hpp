
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_SMP_CPU_HPP
#define POPCORN_KERNEL_SRC_SMP_CPU_HPP

#include <popcorn_prelude.h>

class Cpu {
public:
	usize cpu_id();
};

extern cpu_local Cpu *local_cpu;

#endif   //POPCORN_KERNEL_SRC_SMP_CPU_HPP
