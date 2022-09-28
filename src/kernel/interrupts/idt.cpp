/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "idt.hpp"

using namespace idt;

entry::entry() {
	this->pointer_low = 0;
	this->segment_selector = 0;
	this->ist = 0;
	this->type = 0;
	this->_1 = 0;
	this->dpl = 0;
	this->present = 0;
	this->pointer_middle = 0;
	this->pointer_high = 0;
	this->_2 = 0;
}
