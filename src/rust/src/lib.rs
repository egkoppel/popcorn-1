#![no_std]
#![feature(asm)]
#![feature(ptr_internals)]

#[macro_use]
extern crate modular_bitfield;

#[macro_use]
mod stdio;
mod memory;

use core::panic::PanicInfo;
use crate::memory::frame_alloc::*;
use crate::memory::paging::PAGE_TABLE;

/// This function is called on panic.
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
	eprintln!("{}", info);
	loop {}
}

#[no_mangle]
pub extern "C" fn rust_test(alloc: &mut BumpAllocator) {
	let v = memory::Page::containing_address(memory::VirtualAddress(0xdeadbeef));
	let a = PAGE_TABLE.lock().translate_page(v);
	println!("Translate {:x?} -> {:?}", v, a);

	PAGE_TABLE.lock().mapper().map_page(v, alloc);
	let a = PAGE_TABLE.lock().translate_address(memory::VirtualAddress(0xdeadbeef));
	println!("Translate {:x?} -> {:x?}", v, a);

	let v2 = memory::Page::containing_address(memory::VirtualAddress(0xdeadbeef));
	PAGE_TABLE.lock().mapper().unmap_page(v2, alloc);
	let a = PAGE_TABLE.lock().translate_page(v2);
	println!("Translate {:x?} -> {:x?}", v2, a);

	loop {}
}