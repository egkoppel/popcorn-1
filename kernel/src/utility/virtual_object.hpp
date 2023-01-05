
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_UTILITY_VIRTUAL_OBJECT_HPP
#define POPCORN_KERNEL_SRC_UTILITY_VIRTUAL_OBJECT_HPP

#include <concepts>
#include <new>
#include <utility>

namespace utility {
	template<class T, class U> class VirtualObject {
	public:
		VirtualObject()                     = default;
		VirtualObject(const VirtualObject&) = delete;
		VirtualObject(VirtualObject&&)      = delete;

		template<class... Args2> explicit VirtualObject(U *new_parent, const U& rhs, Args2&&...args2) {
			alignas(alignof(T)) char buffer[sizeof(T)];   // Prevents ctor/dtor being auto-called
			new (buffer) T(std::move(rhs.materialise(*this)));
			alignas(alignof(T)) char buffer_new[sizeof(T)];
			new (buffer_new) T(std::forward<Args2>(args2)..., *reinterpret_cast<const T *>(buffer));
			new_parent->virtualise(*this, reinterpret_cast<const T *>(buffer_new));
		}

		void dtor(U *parent) {
			alignas(alignof(T)) char buffer[sizeof(T)];   // Prevents ctor/dtor being auto-called
			new (buffer) T(std::move(parent->materialise(*this)));
			reinterpret_cast<T *>(buffer)->~T();
		}
	};
}   // namespace utility

#endif   //POPCORN_KERNEL_SRC_UTILITY_VIRTUAL_OBJECT_HPP
