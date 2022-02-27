pub mod tables;
pub mod mapper;

use core::ptr::Unique;
use spin::Mutex;
use core::arch::asm;

use self::{tables::{PageTable, Level4}, mapper::Mapper};
use super::{Page, Frame, VirtualAddress, PhysicalAddress, frame_alloc::Allocator};

pub static PAGE_TABLE: Mutex<ActivePageTable> = Mutex::new(unsafe { ActivePageTable::new() });

fn flush_tlb() {
	unsafe {
		asm!("mov rax, cr3
		  mov cr3, rax", out("rax") _);
	}
}

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

	pub fn with_inactive_table<F>(&mut self, inactive: InactivePageTable, f: F, allocator: &mut dyn Allocator) where F: (Fn(Mapper, &mut dyn Allocator) -> ()) {
		let mut backup_table_addr: u64;
		unsafe {
			asm!("mov {}, cr3", out(reg) backup_table_addr);
		}
		backup_table_addr &= 0xfffffffffffff000;
		self.mapper().map_page_to(Page::containing_address(VirtualAddress(0xFFFFFF80cafebabe)), Frame::with_address(PhysicalAddress(backup_table_addr)), allocator);

		self.p4_as_mut()[510].overwrite_frame(inactive.p4);
		flush_tlb();
		f(self.mapper(), allocator);

		old_p4[510].overwrite_frame(Frame::containing_address(PhysicalAddress(backup_table_addr)));
		flush_tlb();
		self.mapper().unmap_page_no_free(Page::containing_address(VirtualAddress(0xFFFFFF80cafebabe)));
	}
}

pub struct InactivePageTable {
	p4: Frame
}

impl InactivePageTable {
	pub fn new(allocator: &mut dyn Allocator) -> InactivePageTable {
		let p4_addr = 0xFFFFFF80cafeb000u64;

		let p4_frame = allocator.allocate_frame().unwrap();
		PAGE_TABLE.lock().mapper().map_page_to(Page::with_address(VirtualAddress(p4_addr)), p4_frame.clone(), allocator);

		let p4 = unsafe { PageTable::<Level4>::new_from_addr(p4_addr) };
		p4.clear();
		p4[510].set_frame(p4_frame.clone());

		PAGE_TABLE.lock().mapper().unmap_page_no_free(Page::with_address(VirtualAddress(p4_addr)));

		return InactivePageTable {
			p4: p4_frame
		}
	}
}
