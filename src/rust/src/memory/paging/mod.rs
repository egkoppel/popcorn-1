pub mod tables;
pub mod mapper;

use core::ptr::Unique;
use spin::Mutex;

use self::{tables::{PageTable, Level4}, mapper::Mapper};
use super::{Page, Frame, VirtualAddress, PhysicalAddress};

pub static PAGE_TABLE: Mutex<ActivePageTable> = Mutex::new(unsafe { ActivePageTable::new() });

pub struct ActivePageTable {
	p4: Unique<PageTable<Level4>>
}

impl ActivePageTable {
	pub const unsafe fn new() -> ActivePageTable {
		return ActivePageTable {
			p4: Unique::new_unchecked(0o1777777767767767760000 as *mut PageTable<Level4>)
		};
	}

	pub fn p4_as_ref(&self) -> &PageTable<Level4> {
		return unsafe { self.p4.as_ref() };
	}

	pub fn p4_as_mut(&mut self) -> &mut PageTable<Level4> {
		return unsafe { self.p4.as_mut() };
	}

	pub fn mapper(&mut self) -> Mapper {
		return Mapper::new(self.p4_as_mut());
	}

	pub fn translate_page(&self, page: Page) -> Option<Frame> {
		return self.p4_as_ref().get_child_table(page.start_address().p4_index())
			.and_then(|p3| p3.get_child_table(page.start_address().p3_index()))
			.and_then(|p2| p2.get_child_table(page.start_address().p2_index()))
			.and_then(|p1| p1[page.start_address().p1_index()].get_frame());
	}

	pub fn translate_address(&self, addr: VirtualAddress) -> Option<PhysicalAddress> {
		return self.translate_page(Page::containing_address(addr))
			.and_then(|frame| Some(frame.start_address() + addr.p1_offset()));
	}
}