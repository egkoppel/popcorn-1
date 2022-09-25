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
