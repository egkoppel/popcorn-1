
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STRUCTURES_LOCK_FREE_QUEUE_HPP
#define HUGOS_KERNEL_SRC_STRUCTURES_LOCK_FREE_QUEUE_HPP

namespace structures::lock_free {
	template<class T> class queue {
	public:
		void push(T&&);
		bool pop(T *);
	};
}   // namespace structures::lock_free

#endif   //HUGOS_KERNEL_SRC_STRUCTURES_LOCK_FREE_QUEUE_HPP
