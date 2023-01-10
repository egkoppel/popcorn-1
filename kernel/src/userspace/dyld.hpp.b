/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by Eliyahu Gluschove-Koppel on 21/09/2022.
//

#ifndef HUGOS_DYLD_HPP_B
#define HUGOS_DYLD_HPP_B

#include "elf.hpp"

int dyld(Elf64::Elf64File file);

#endif //HUGOS_DYLD_HPP_B
