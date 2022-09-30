/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use core::marker::PhantomData;
use core::ops::{Index, IndexMut};

use crate::memory::frame_alloc::Allocator;
use crate::memory::{PhysicalAddress, Frame};

pub trait TableLevel {}

pub trait HierarchicalTableLevel: TableLevel {
	type ChildLevel: TableLevel;
}

pub enum Level1 {}

pub enum Level2 {}

pub enum Level3 {}

pub enum Level4 {}

impl TableLevel for Level1 {}

impl TableLevel for Level2 {}

impl TableLevel for Level3 {}

impl TableLevel for Level4 {}

impl HierarchicalTableLevel for Level2 {
	type ChildLevel = Level1;
}

impl HierarchicalTableLevel for Level3 {
	type ChildLevel = Level2;
}

impl HierarchicalTableLevel for Level4 {
	type ChildLevel = Level3;
}

use modular_bitfield::specifiers::*;

#[bitfield(bytes = 8)]
#[repr(C, u64)]
#[derive(Debug, Clone)]
pub struct Entry {
	pub present: bool,
	pub writeable: bool,
	pub user_accessible: bool,
	pub write_through: bool,
	pub cache_disabled: bool,
	pub accessed: bool,
	pub dirty: bool,
	pub huge: bool,
	pub global: bool,
	no_map: bool,
	#[skip] __: B2,
	internal_address: B40,
	#[skip] __: B11,
	pub no_execute: bool,
}

impl Entry {
	pub fn set_address(&mut self, frame: Frame) -> Result<(), i8> {
		if self.present() {
			println!("Page already mapped");
			return Err(-1);
		}
		if self.no_map() {
			println!("Attempted to map page that was marked for never map");
			return Err(-2);
		}
		self.set_present(true);
		self.overwrite_address(frame);
		return Ok(());
	}

	pub fn overwrite_address(&mut self, frame: Frame) {
		serialprintln!("setting addr to 0x{:x}", frame.start_address().0 >> 12);
		self.set_internal_address(frame.start_address().0 >> 12);
	}

	pub fn get_address(&self) -> Option<Frame> {
		if self.present() {
			return Some(Frame::with_address(PhysicalAddress(self.internal_address() << 12)));
		} else { return None; }
	}

	pub fn clear(&mut self) {
		*self = 0.into();
	}

	pub fn set_flags(&mut self, flags: EntryFlags) {
		self.set_writeable(flags.writeable);
		self.set_user_accessible(flags.user_accessible);
		self.set_write_through(flags.write_through);
		self.set_cache_disabled(flags.cache_disabled);
		self.set_accessed(flags.accessed);
		self.set_dirty(flags.dirty);
		self.set_huge(flags.huge);
		self.set_global(flags.global);
		self.set_no_execute(flags.no_execute);
	}

	pub fn add_flags(&mut self, flags: EntryFlags) {
		self.set_writeable(self.writeable() || flags.writeable);
		self.set_user_accessible(self.user_accessible() || flags.user_accessible);
	}

	pub fn mark_for_never_mapped(&mut self) {
		assert!(!self.present(), "Cannot mark for never map if already mapped");
		self.set_no_map(true);
	}

	pub fn unmark_for_never_mapped(&mut self) {
		assert!(self.no_map(), "Cannot unmark for never map if not marked for no map");
		self.set_no_map(false);
	}
}

#[repr(C, packed)]
pub struct PageTable<L> where L: TableLevel {
	entries: [Entry; 512],
	level: PhantomData<L>,
}

impl<L> PageTable<L> where L: TableLevel {
	pub fn clear(&mut self) -> &mut Self {
		for entry in &mut self.entries {
			entry.clear();
		}
		return self;
	}
}

impl PageTable<Level4> {
	pub unsafe fn new_from_addr<'a>(addr: u64) -> &'a mut PageTable<Level4> {
		assert_eq!(addr & 0xfff, 0, "Page table is not 4k aligned");
		return &mut *(addr as *mut _);
	}
}

impl<L> Index<u64> for PageTable<L> where L: TableLevel {
	type Output = Entry;
	fn index(&self, index: u64) -> &Self::Output {
		return &self.entries[index as usize];
	}
}

impl<L> IndexMut<u64> for PageTable<L> where L: TableLevel {
	fn index_mut(&mut self, index: u64) -> &mut Self::Output {
		return &mut self.entries[index as usize];
	}
}

impl<L> PageTable<L> where L: HierarchicalTableLevel {
	pub fn get_child_table(&self, index: u64) -> Option<&PageTable<L::ChildLevel>> {
		assert!(index < 512, "Attempt to access page table entry {}", index);
		if !self[index].present() || self[index].huge() { return None; } else {
			return Some(unsafe {
				&*(((self as *const _ as u64) << 9 | (index as u64) << 12 | 0o1777770000000000000000) as *const PageTable<L::ChildLevel>)
			});
		}
	}

	pub fn get_child_table_mut(&mut self, index: u64) -> Option<&mut PageTable<L::ChildLevel>> {
		assert!(index < 512, "Attempt to access page table entry {}", index);
		if !self[index].present() || self[index].huge() { return None; } else {
			return Some(unsafe {
				&mut *(((self as *mut _ as u64) << 9 | (index as u64) << 12 | 0o1777770000000000000000) as *mut PageTable<L::ChildLevel>)
			});
		}
	}

	pub fn get_child_table_or_new(&mut self, index: u64, frame_allocator: &mut dyn Allocator) -> &mut PageTable<L::ChildLevel> {
		let child_table = self.get_child_table_mut(index);
		if child_table.is_none() {
			let new_table = frame_allocator.allocate_frame().expect("Unable to allocate frame for new page table");
			self[index].set_address(new_table);
			self[index].set_writeable(true);
			return self.get_child_table_mut(index).unwrap().clear();
		}
		return self.get_child_table_mut(index).unwrap();
	}
}

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub struct EntryFlags {
	pub writeable: bool,
	pub user_accessible: bool,
	pub write_through: bool,
	pub cache_disabled: bool,
	pub accessed: bool,
	pub dirty: bool,
	pub huge: bool,
	pub global: bool,
	pub no_execute: bool,
}

mod c_api {
	use super::super::PAGE_TABLE;
	use super::EntryFlags;
	use crate::memory::{VirtualAddress, Page};
	use core::ops::IndexMut;
	use crate::CAllocatorVtable;

	#[no_mangle]
	extern "C" fn set_entry_flags_for_address(addr: VirtualAddress, flags: EntryFlags) -> i32 {
		let page = Page::with_address(addr);
		let mut p4 = PAGE_TABLE.lock();
		let entry = p4.p4_as_mut().get_child_table_mut(page.start_address().p4_index())
		              .and_then(|p3| p3.get_child_table_mut(page.start_address().p3_index()))
		              .and_then(|p2| p2.get_child_table_mut(page.start_address().p2_index()))
		              .and_then(|p1| Some(p1.index_mut(page.start_address().p1_index())));
		if let Some(entry) = entry {
			entry.set_flags(flags);
			return 0;
		} else { return -1; }
	}

	#[no_mangle]
	extern "C" fn mark_for_no_map(addr: VirtualAddress, allocator: Option<&mut CAllocatorVtable>) {
		let a = allocator.unwrap();
		let page = Page::with_address(addr);
		let mut p4 = PAGE_TABLE.lock();
		p4.p4_as_mut().get_child_table_or_new(page.start_address().p4_index(), a)
		  .get_child_table_or_new(page.start_address().p3_index(), a)
		  .get_child_table_or_new(page.start_address().p2_index(), a)
			[page.start_address().p1_index()]
			.mark_for_never_mapped();
	}

	#[no_mangle]
	extern "C" fn unmark_for_no_map(addr: VirtualAddress) {
		let page = Page::with_address(addr);
		let mut p4 = PAGE_TABLE.lock();
		let entry = p4.p4_as_mut().get_child_table_mut(page.start_address().p4_index())
		              .and_then(|p3| p3.get_child_table_mut(page.start_address().p3_index()))
		              .and_then(|p2| p2.get_child_table_mut(page.start_address().p2_index()))
		              .and_then(|p1| Some(p1.index_mut(page.start_address().p1_index())));
		entry.unwrap().unmark_for_never_mapped();
	}
}
