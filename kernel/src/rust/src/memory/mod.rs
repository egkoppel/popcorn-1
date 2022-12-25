/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use core::{fmt, ops};

pub mod frame_alloc;
pub mod paging;

#[repr(transparent)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct VirtualAddress(pub u64);

impl VirtualAddress {
	pub const fn p4_index(&self) -> u64 {
		return (self.0 >> 39) & 0o777;
	}

	pub const fn p3_index(&self) -> u64 {
		return (self.0 >> 30) & 0o777;
	}

	pub const fn p2_index(&self) -> u64 {
		return (self.0 >> 21) & 0o777;
	}

	pub const fn p1_index(&self) -> u64 {
		return (self.0 >> 12) & 0o777;
	}

	pub const fn p1_offset(&self) -> u64 {
		return self.0 & 0o7777;
	}
}

impl ops::Add<u64> for VirtualAddress {
	type Output = Self;

	fn add(self, rhs: u64) -> Self {
		return VirtualAddress(self.0 + rhs);
	}
}

impl ops::Sub<u64> for VirtualAddress {
	type Output = Self;

	fn sub(self, rhs: u64) -> Self {
		return VirtualAddress(self.0 - rhs);
	}
}

#[repr(transparent)]
#[derive(Debug, Copy, Clone)]
pub struct PhysicalAddress(pub u64);

impl ops::Add<u64> for PhysicalAddress {
	type Output = Self;

	fn add(self, rhs: u64) -> Self {
		return PhysicalAddress(self.0 + rhs);
	}
}

impl ops::Sub<u64> for PhysicalAddress {
	type Output = Self;

	fn sub(self, rhs: u64) -> Self {
		return PhysicalAddress(self.0 - rhs);
	}
}

#[derive(Clone, Copy)]
pub struct Page {
	number: u64,
}

impl Page {
	pub const fn containing_address(addr: VirtualAddress) -> Page {
		return Page {
			number: addr.0 / 4096
		};
	}

	pub fn with_address(addr: VirtualAddress) -> Page {
		assert_eq!(addr.0 & 0xfff, 0, "Page is not 4k aligned");
		return Page {
			number: addr.0 / 4096
		};
	}

	pub fn start_address(&self) -> VirtualAddress {
		return VirtualAddress(self.number * 4096);
	}
}

impl fmt::Debug for Page {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		let _end = (self.number + 1).checked_mul(4096);
		f.debug_struct("Page")
		 .field("start", &(self.number * 4096))
		 .field("end", {
			 if let Some(end) = _end.as_ref() {
				 end
			 } else {
				 &0
			 }
		 })
		 .finish()
	}
}

#[derive(PartialEq, Eq, PartialOrd, Ord)]
pub struct Frame {
	number: u64,
}

impl Frame {
	pub fn containing_address(addr: PhysicalAddress) -> Frame {
		return Frame {
			number: addr.0 / 4096
		};
	}

	pub fn with_address(addr: PhysicalAddress) -> Frame {
		assert_eq!(addr.0 & 0xfff, 0, "Frame is not 4k aligned");
		return Frame {
			number: addr.0 / 4096
		};
	}

	pub fn start_address(&self) -> PhysicalAddress {
		return PhysicalAddress(self.number * 4096);
	}

	pub(self) fn clone(&self) -> Frame {
		return Frame { number: self.number };
	}
}

impl fmt::Debug for Frame {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		f.debug_struct("Frame")
		 .field("start", &(self.number * 4096))
		 .field("end", &((self.number + 1) * 4096))
		 .finish()
	}
}
