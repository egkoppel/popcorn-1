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
#[derive(Debug)]
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
	#[skip] __: B3,
	internal_address: B40,
	#[skip] __: B11,
	pub no_execute: bool
} 

impl Entry {
	pub fn set_address(&mut self, frame: Frame) {
		assert!(!self.present(), "Entry is already mapped");
		self.set_present(true);
		self.overwrite_address(frame);
	}

	pub fn overwrite_address(&mut self, frame: Frame) {
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
}

#[repr(C, packed)]
pub struct PageTable<L> where L:TableLevel {
	entries: [Entry; 512],
	level: PhantomData<L>
}

impl<L> PageTable<L> where L:TableLevel {
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

impl<L> Index<u64> for PageTable<L> where L:TableLevel {
	type Output = Entry;
	fn index(&self, index: u64) -> &Self::Output {
		return &self.entries[index as usize];
	}
}

impl<L> IndexMut<u64> for PageTable<L> where L:TableLevel {
	fn index_mut(&mut self, index: u64) -> &mut Self::Output {
		return &mut self.entries[index as usize];
	}
}

impl<L> PageTable<L> where L: HierarchicalTableLevel {
	pub fn get_child_table(&self, index: u64) -> Option<&PageTable<L::ChildLevel>> {
		assert!(index < 512, "Attempt to access page table entry {}", index);
		if !self[index].present() || self[index].huge() { return None; }
		else {
			return Some(unsafe {
				&*(((self as *const _ as u64) << 9 | (index as u64) << 12 | 0o1777770000000000000000) as *const PageTable<L::ChildLevel>)
			});
		}
	}

	pub fn get_child_table_mut(&mut self, index: u64) -> Option<&mut PageTable<L::ChildLevel>> {
		assert!(index < 512, "Attempt to access page table entry {}", index);
		if !self[index].present() || self[index].huge() { return None; }
		else {
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
			return self.get_child_table_mut(index).unwrap().clear();
		} 
		return self.get_child_table_mut(index).unwrap();
	}
}

mod c_api {
	use super::super::PAGE_TABLE;
	use crate::memory::{VirtualAddress, Page};
	use core::ops::IndexMut;

	#[repr(C)]
	struct EntryFlags {
		pub writeable: bool,
		pub user_accessible: bool,
		pub write_through: bool,
		pub cache_disabled: bool,
		pub accessed: bool,
		pub dirty: bool,
		pub huge: bool,
		pub global: bool,
		pub no_execute: bool
	}

	#[no_mangle] extern "C" fn set_entry_flags_for_address(addr: VirtualAddress, flags: EntryFlags) -> i32 {
		let page = Page::with_address(addr);
		let mut p4 = PAGE_TABLE.lock();
		let entry = p4.p4_as_mut().get_child_table_mut(page.start_address().p4_index())
			.and_then(|p3| p3.get_child_table_mut(page.start_address().p3_index()))
			.and_then(|p2| p2.get_child_table_mut(page.start_address().p2_index()))
			.and_then(|p1| Some(p1.index_mut(page.start_address().p1_index())));
		if let Some(entry) = entry {
			entry.set_writeable(flags.writeable);
			entry.set_user_accessible(flags.user_accessible);
			entry.set_write_through(flags.write_through);
			entry.set_cache_disabled(flags.cache_disabled);
			entry.set_accessed(flags.accessed);
			entry.set_dirty(flags.dirty);
			entry.set_huge(flags.huge);
			entry.set_global(flags.global);
			entry.set_no_execute(flags.no_execute);
			return 0;
		} else { return -1; }
	}
}
