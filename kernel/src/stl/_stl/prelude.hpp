
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_STL__STL_PRELUDE_HPP
#define HUGOS_KERNEL_SRC_STL__STL_PRELUDE_HPP

#ifdef HUGOS_TEST
	#define HUGOS_STL_BEGIN_NAMESPACE namespace hugos_std {
	#define HUGOS_STL_END_NAMESPACE   }
#else
	#define HUGOS_STL_BEGIN_NAMESPACE namespace std {
	#define HUGOS_STL_END_NAMESPACE   }
#endif

#define HUGOS_STL_PRIVATE_NAMESPACE       detail

#define HUGOS_STL_BEGIN_PRIVATE_NAMESPACE namespace HUGOS_STL_PRIVATE_NAMESPACE {
#define HUGOS_STL_END_PRIVATE_NAMESPACE   }

#endif   //HUGOS_KERNEL_SRC_STL__STL_PRELUDE_HPP
