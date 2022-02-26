use crate::memory::{Page, Frame};
use crate::memory::frame_alloc::Allocator;

use super::tables::{PageTable, Level4};
use core::arch::asm;

pub struct Mapper<'a> {
	p4: &'a mut PageTable<Level4>
}

impl Mapper<'_> {
	pub fn new<'a>(p4: &'a mut PageTable<Level4>) -> Mapper<'a> {
		return Mapper { p4: p4 };
	}

	pub fn map_page_to(&mut self, page: Page, frame: Frame, allocator: &mut dyn Allocator) {
		let p1 = self.p4
			.get_child_table_or_new(page.start_address().p4_index(), allocator)
			.get_child_table_or_new(page.start_address().p3_index(), allocator)
			.get_child_table_or_new(page.start_address().p2_index(), allocator);
		p1[page.start_address().p1_index()].set_frame(frame);
	}

	pub fn map_page(&mut self, page: Page, allocator: &mut dyn Allocator) {
		let frame_ = allocator.allocate_frame();
		assert!(frame_.is_ok(), "Couldn't allocate frame to map to {:?}", page);
		let frame = frame_.unwrap();
		self.map_page_to(page, frame, allocator);
	}

	pub fn unmap_page(&mut self, page: Page, allocator: &mut dyn Allocator) {
		let p1 = self.p4
			.get_child_table_mut(page.start_address().p4_index())
			.and_then(|p3| p3.get_child_table_mut(page.start_address().p3_index()))
			.and_then(|p2| p2.get_child_table_mut(page.start_address().p2_index()))
			.expect("Attempted to unmap unmapped address");
		let entry = &mut p1[page.start_address().p1_index()];
		allocator.deallocate_frame(entry.get_frame().unwrap());
		entry.clear();
		unsafe { asm!("invlpg [{}]", in(reg) page.start_address().0); }
	}
}