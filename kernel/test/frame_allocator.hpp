
// Copyright (c) 2022 Eliyahu Gluschove-Koppel.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HUGOS_FRAME_ALLOCATOR_HPP
#define HUGOS_FRAME_ALLOCATOR_HPP

#include <gtest/gtest.h>

#define private public
#define protected public

#include <memory/physical_allocator.hpp>

#undef private
#undef protected

class IAllocatorTest : public ::testing::Test, public ::testing::WithParamInterface<IAllocator> {
	IAllocator *allocator;
};

#endif //HUGOS_FRAME_ALLOCATOR_HPP
