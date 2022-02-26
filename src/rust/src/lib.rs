#![no_std]
#![feature(asm)]
#![feature(ptr_internals)]

#[macro_use]
extern crate modular_bitfield;

use core::panic::PanicInfo;

/// This function is called on panic.
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
	eprintln!("{}", info);
	loop {}
}
