
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_EXCEPTION_HPP
#define HUGOS_KERNEL_SRC_STL__STL_EXCEPTION_HPP

HUGOS_STL_BEGIN_NAMESPACE

class exception {
public:
	exception() noexcept                       = default;
	virtual ~exception()                       = default;
	exception(const exception& other) noexcept = default;
	virtual const char *what() const noexcept  = 0;
};

HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_EXCEPTION_HPP
