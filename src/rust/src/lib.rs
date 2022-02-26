#![no_std]
#![feature(ptr_internals)]

#[macro_use]
extern crate modular_bitfield;

#[macro_use]
mod stdio;
mod memory;

use core::panic::PanicInfo;
use crate::memory::frame_alloc::*;
use crate::memory::paging::{PAGE_TABLE, InactivePageTable};

/// This function is called on panic.
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
	eprintln!("{}", info);
	loop {}
}

#[no_mangle]
pub extern "C" fn rust_test(alloc: &mut BumpAllocator) {
	let v = memory::Page::containing_address(memory::VirtualAddress(0xdeadbeef));
	let a = PAGE_TABLE.lock().mapper().translate_page(v);
	println!("Translate {:x?} -> {:?}", v, a);

	PAGE_TABLE.lock().mapper().map_page(v, alloc);
	let a =  PAGE_TABLE.lock().mapper().translate_page(v);
	println!("Translate {:x?} -> {:x?}", v, a);

	PAGE_TABLE.lock().mapper().unmap_page(v, alloc);
	let a = PAGE_TABLE.lock().mapper().translate_page(v);
	println!("Translate {:x?} -> {:x?}", v, a);

	let inactive = InactivePageTable::new(alloc);
	PAGE_TABLE.lock().with_inactive_table(inactive, |mut mapper, frame_alloc| {
		println!("In inactive table closure");
		let v = memory::Page::containing_address(memory::VirtualAddress(0xdeadbeef));
		let a = mapper.translate_page(v);
		println!("Translate in inactive {:x?} -> {:x?}", v, a);

		mapper.map_page(v, frame_alloc);

		let a = mapper.translate_page(v);
		println!("Translate in inactive {:x?} -> {:x?}", v, a);

		mapper.unmap_page(v, frame_alloc);
		let a = mapper.translate_page(v);
		println!("Translate in inactive {:x?} -> {:x?}", v, a);
	}, alloc);

	loop {}
}