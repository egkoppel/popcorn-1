
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_TYPEINFO_HPP
#define HUGOS_KERNEL_SRC_STL__STL_TYPEINFO_HPP

HUGOS_STL_BEGIN_NAMESPACE
class type_info {
public:
	virtual ~type_info() {}
	bool operator==(const type_info& other) const { return this == &other; }
	bool operator!=(const type_info& other) const { return this != &other; }
	bool before(const type_info& other) const { return this < &other; }
	const char *name() const { return __type_name; }

private:
	const char *__type_name;

protected:
	type_info(const char *name) : __type_name(name) {}
	type_info(const type_info& rhs)            = default;
	type_info& operator=(const type_info& rhs) = default;
};

class bad_cast;

class bad_typeid;
HUGOS_STL_END_NAMESPACE

#endif   //HUGOS_KERNEL_SRC_STL__STL_TYPEINFO_HPP
