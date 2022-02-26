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
#[derive(Copy, Clone, Debug)]
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
	pub fn set_frame(&mut self, frame: Frame) {
		assert!(!self.present(), "Entry is already mapped");
		self.set_present(true);
		self.set_internal_address(frame.start_address().0 >> 12);
	}

	pub fn get_frame(&self) -> Option<Frame> {
		if self.present() {
			return Some(Frame::containing_address(PhysicalAddress(self.internal_address() << 12)));
		} else { return None; }
	}

	pub fn clear(&mut self) {
		*self = 0.into();
	}
}

#[repr(C)]
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
			self[index].set_frame(new_table);
			return self.get_child_table_mut(index).unwrap().clear();
		} 
		return self.get_child_table_mut(index).unwrap();
	}
}
