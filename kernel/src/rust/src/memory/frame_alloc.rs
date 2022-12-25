/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use core::ffi::c_void;

use super::{PhysicalAddress, Frame};

pub trait Allocator {
	fn allocate_frame(&mut self) -> Option<Frame>;
	fn deallocate_frame(&mut self, _addr: Frame) {}
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CAllocatorVtable {
	allocate: extern "C" fn(*mut CAllocatorVtable) -> *mut c_void,
	deallocate: extern "C" fn(*mut CAllocatorVtable, *mut c_void),
}

impl Allocator for CAllocatorVtable {
	fn allocate_frame(&mut self) -> Option<Frame> {
		let addr = (self.allocate)(self as *mut _);
		return Some(Frame::with_address(PhysicalAddress(addr as u64)));
	}

	fn deallocate_frame(&mut self, addr: Frame) {
		(self.deallocate)(self as *mut _, addr.start_address().0 as *mut c_void);
	}
}
