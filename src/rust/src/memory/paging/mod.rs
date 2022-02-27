pub mod tables;
pub mod mapper;

use core::ptr::Unique;
use spin::Mutex;
use core::arch::asm;

use self::{tables::{PageTable, Level4}, mapper::Mapper};
use super::{Page, Frame, VirtualAddress, PhysicalAddress, frame_alloc::Allocator};

pub static PAGE_TABLE: Mutex<ActivePageTable> = Mutex::new(unsafe { ActivePageTable::new() });
const magicpage: Page = Page::containing_address(VirtualAddress(0xFFFFFF80cafebabe));

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

	fn map_inactive_table(&mut self, inactive: InactivePageTable, allocator: &mut dyn Allocator) -> PhysicalAddress {
		let mut backup_table_addr: u64;
		unsafe {
			asm!("mov {}, cr3", out(reg) backup_table_addr);
		}
		backup_table_addr &= 0xfffffffffffff000;
		self.mapper().map_page_to(magicpage, Frame::with_address(PhysicalAddress(backup_table_addr)), allocator);

		self.p4_as_mut()[510].overwrite_address(inactive.p4);
		flush_tlb();

		return PhysicalAddress(backup_table_addr);
	}

	fn unmap_inactive_table(&mut self, backup_table_addr: PhysicalAddress) {
		let old_p4 = unsafe { PageTable::<Level4>::new_from_addr(magicpage.start_address().0) };
		old_p4[510].overwrite_address(Frame::containing_address(backup_table_addr));
		flush_tlb();
		self.mapper().unmap_page_no_free(magicpage);
	}

	pub fn with_inactive_table<F>(&mut self, inactive: InactivePageTable, f: F, allocator: &mut dyn Allocator) where F: (Fn(Mapper, &mut dyn Allocator) -> ()) {
		let backup = self.map_inactive_table(inactive, allocator);

		f(self.mapper(), allocator);

		self.unmap_inactive_table(backup);
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
		p4[510].set_address(p4_frame.clone());

		PAGE_TABLE.lock().mapper().unmap_page_no_free(Page::with_address(VirtualAddress(p4_addr)));

		return InactivePageTable {
			p4: p4_frame
		}
	}
}

mod c_api {
	use crate::{PAGE_TABLE, InactivePageTable};
	use crate::memory::frame_alloc::CAllocatorVtable;
	use crate::memory::{Frame, Page, PhysicalAddress, VirtualAddress};

	#[no_mangle] extern "C" fn map_page(addr: VirtualAddress, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		if allocator.is_none() { return -1; }
		PAGE_TABLE.lock().mapper().map_page(Page::with_address(addr), allocator.unwrap());
		return 0;
	}

	#[no_mangle] extern "C" fn map_page_to(virt_addr: VirtualAddress, phys_addr: PhysicalAddress, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		if allocator.is_none() { return -1; }
		PAGE_TABLE.lock().mapper().map_page_to(Page::with_address(virt_addr), Frame::with_address(phys_addr), allocator.unwrap());
		return 0;
	}

	#[no_mangle] extern "C" fn unmap_page(addr: VirtualAddress, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		if allocator.is_none() { return -1; }
		PAGE_TABLE.lock().mapper().unmap_page(Page::with_address(addr), allocator.unwrap());
		return 0;
	}

	#[no_mangle] extern "C" fn unmap_page_no_free(addr: VirtualAddress) {
		PAGE_TABLE.lock().mapper().unmap_page_no_free(Page::with_address(addr));
	}

	#[no_mangle] extern "C" fn translate_page(addr: VirtualAddress, ret: Option<&mut u64>) -> i32 {
		let frame = PAGE_TABLE.lock().mapper().translate_page(Page::with_address(addr));
		if frame.is_none() { return -1; }
		if ret.is_some() { *ret.unwrap() = frame.unwrap().start_address().0; }
		return 0;
	}

	#[no_mangle] extern "C" fn translate_addr(addr: VirtualAddress, ret: Option<&mut u64>) -> i32 {
		let paddr = PAGE_TABLE.lock().mapper().translate_address(addr);
		if paddr.is_none() { return -1; }
		if ret.is_some() { *ret.unwrap() = paddr.unwrap().0; }
		return 0;
	}

	#[repr(C)]
	struct MapperCtx {
		backup_table_addr: PhysicalAddress
	}

	#[no_mangle] extern "C" fn mapper_ctx_begin(inactive_table_frame: PhysicalAddress, allocator: Option<&mut CAllocatorVtable>) -> MapperCtx {
		let backup_addr = PAGE_TABLE.lock().map_inactive_table(InactivePageTable {
			p4: Frame::with_address(inactive_table_frame)
		}, allocator.unwrap());

		return MapperCtx {
			backup_table_addr: backup_addr
		};
	}

	#[no_mangle] extern "C" fn mapper_ctx_end(ctx: MapperCtx) {
		PAGE_TABLE.lock().unmap_inactive_table(ctx.backup_table_addr);
	}

	#[no_mangle] extern "C" fn create_p4_table(allocator: Option<&mut CAllocatorVtable>) -> PhysicalAddress {
		return InactivePageTable::new(allocator.unwrap()).p4.start_address();
	}
}
