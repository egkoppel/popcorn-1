/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

pub mod tables;
pub mod mapper;

use core::ptr::Unique;
use spin::Mutex;
use core::arch::asm;

use self::{tables::{PageTable, Level4, EntryFlags}, mapper::Mapper};
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
	p4: Unique<PageTable<Level4>>,
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

		serialprintln!("RSpage - Map inactive table - {:x?} -> {:x?}", magicpage, Frame::with_address(PhysicalAddress(backup_table_addr)));

		self.mapper().map_page_to(magicpage, Frame::with_address(PhysicalAddress(backup_table_addr)), EntryFlags {
			writeable: true,
			user_accessible: false,
			write_through: false,
			cache_disabled: false,
			accessed: false,
			dirty: false,
			huge: false,
			global: false,
			no_execute: true,
		}, allocator);

		serialprintln!("RSpage - p4[510] currently mapped to {:x?}", self.p4_as_ref()[510].get_address());
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
	p4: Frame,
}

impl InactivePageTable {
	pub fn new(allocator: &mut dyn Allocator) -> InactivePageTable {
		let p4_frame = allocator.allocate_frame().unwrap();
		serialprintln!("Mapping new p4 table");
		PAGE_TABLE.lock().mapper().map_page_to(magicpage, p4_frame.clone(), EntryFlags {
			writeable: true,
			user_accessible: false,
			write_through: false,
			cache_disabled: false,
			accessed: false,
			dirty: false,
			huge: false,
			global: false,
			no_execute: true,
		}, allocator).unwrap();

		let p4 = unsafe { PageTable::<Level4>::new_from_addr(magicpage.start_address().0) };

		p4.clear();
		p4[510].set_address(p4_frame.clone());
		p4[510].set_writeable(true);
		p4[510].set_no_execute(true);

		PAGE_TABLE.lock().mapper().unmap_page_no_free(magicpage);

		return InactivePageTable {
			p4: p4_frame
		};
	}
}

mod c_api {
	use crate::{PAGE_TABLE, InactivePageTable};
	use crate::memory::frame_alloc::CAllocatorVtable;
	use crate::memory::{Frame, Page, PhysicalAddress, VirtualAddress};
	use crate::memory::paging::magicpage;
	use crate::memory::paging::tables::{Level4, PageTable};

	use super::tables::EntryFlags;

	#[no_mangle]
	extern "C" fn map_page(addr: VirtualAddress, flags: EntryFlags, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		if allocator.is_none() { return -5; }
		return PAGE_TABLE.lock().mapper().map_page(Page::with_address(addr), flags, allocator.unwrap()).map_or_else(|e| e, |o| 0).into();
		;
	}

	#[no_mangle]
	extern "C" fn map_page_to(virt_addr: VirtualAddress, phys_addr: PhysicalAddress, flags: EntryFlags, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		if allocator.is_none() { return -5; }
		return PAGE_TABLE.lock().mapper().map_page_to(Page::with_address(virt_addr), Frame::with_address(phys_addr), flags, allocator.unwrap()).map_or_else(|e| e, |o| 0).into();
	}

	#[no_mangle]
	extern "C" fn unmap_page(addr: VirtualAddress, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		if allocator.is_none() { return -5; }
		PAGE_TABLE.lock().mapper().unmap_page(Page::with_address(addr), allocator.unwrap());
		return 0;
	}

	#[no_mangle]
	extern "C" fn unmap_page_no_free(addr: VirtualAddress) {
		PAGE_TABLE.lock().mapper().unmap_page_no_free(Page::with_address(addr));
	}

	#[no_mangle]
	extern "C" fn translate_page(addr: VirtualAddress, ret: Option<&mut u64>) -> i32 {
		let frame = PAGE_TABLE.lock().mapper().translate_page(Page::with_address(addr));
		if frame.is_none() { return -1; }
		if ret.is_some() { *ret.unwrap() = frame.unwrap().start_address().0; }
		return 0;
	}

	#[no_mangle]
	extern "C" fn translate_addr(addr: VirtualAddress, ret: Option<&mut u64>) -> i32 {
		let paddr = PAGE_TABLE.lock().mapper().translate_address(addr);
		if paddr.is_none() { return -1; }
		if ret.is_some() { *ret.unwrap() = paddr.unwrap().0; }
		return 0;
	}

	#[repr(C)]
	struct MapperCtx {
		backup_table_addr: PhysicalAddress,
	}

	#[no_mangle]
	extern "C" fn mapper_ctx_begin(inactive_table_frame: PhysicalAddress, allocator: Option<&mut CAllocatorVtable>) -> MapperCtx {
		serialprintln!("C/RS bridge - mapper_ctx_begin - new table at {:x?}", Frame::with_address(inactive_table_frame));
		let backup_addr = PAGE_TABLE.lock().map_inactive_table(InactivePageTable {
			p4: Frame::with_address(inactive_table_frame)
		}, allocator.unwrap());

		return MapperCtx {
			backup_table_addr: backup_addr
		};
	}

	#[no_mangle]
	extern "C" fn mapper_ctx_end(ctx: MapperCtx) {
		PAGE_TABLE.lock().unmap_inactive_table(ctx.backup_table_addr);
	}

	#[no_mangle]
	extern "C" fn create_p4_table(allocator: Option<&mut CAllocatorVtable>) -> PhysicalAddress {
		return InactivePageTable::new(allocator.unwrap()).p4.start_address();
	}

	#[no_mangle]
	extern "C" fn map_kernel_from_current_into(p4_table: PhysicalAddress, allocator: Option<&mut CAllocatorVtable>) -> i32 {
		PAGE_TABLE.lock().mapper().map_page_to(magicpage, Frame::with_address(p4_table), EntryFlags {
			writeable: true,
			user_accessible: false,
			write_through: false,
			cache_disabled: false,
			accessed: false,
			dirty: false,
			huge: false,
			global: false,
			no_execute: true,
		}, allocator.unwrap()).unwrap();

		let p4 = unsafe { PageTable::<Level4>::new_from_addr(magicpage.start_address().0) };

		// Kernel stack allocator allocation area
		p4[509] = PAGE_TABLE.lock().p4_as_ref()[509].clone();
		// Kernel .text and data area
		p4[511] = PAGE_TABLE.lock().p4_as_ref()[511].clone();

		PAGE_TABLE.lock().mapper().unmap_page_no_free(magicpage);

		return 0;
	}
}
