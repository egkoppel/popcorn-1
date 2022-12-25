
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_INITIALISATION_HPP
#define HUGOS_INITIALISATION_HPP

namespace arch {
	int arch_specific_early_init();
	int arch_specific_late_init();
}   // namespace arch

#endif   //HUGOS_INITIALISATION_HPP
