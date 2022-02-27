use core::ffi::c_void;

use super::{PhysicalAddress, Frame};

pub trait Allocator {
	fn allocate_frame(&mut self) -> Option<Frame>;
	fn deallocate_frame(&mut self, _addr: Frame) {}
}

#[repr(C)]
pub struct BumpAllocator {
	vtable: *const c_void,
	next_alloc: PhysicalAddress,
	kernel_start: PhysicalAddress,
	kernel_end: PhysicalAddress,
	multiboot_start: PhysicalAddress,
	multiboot_end: PhysicalAddress,
	multiboot_memory_map_tag: *const c_void
}

extern "C" {
	fn frame_bump_alloc_allocate(state: *mut BumpAllocator) -> *mut c_void;
}

impl Allocator for BumpAllocator {
	fn allocate_frame(&mut self) -> Option<Frame> {
		unsafe {
			let new_frame = frame_bump_alloc_allocate(self as *mut _);
			return Some(Frame::with_address(PhysicalAddress(new_frame as u64)));
		}
	}
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CAllocatorVtable {
	allocate: extern "C" fn(*mut CAllocatorVtable) -> *mut c_void,
	deallocate: extern "C" fn(*mut CAllocatorVtable, *mut c_void)
}

impl Allocator for CAllocatorVtable {
	fn allocate_frame(&mut self) -> Option<Frame> {
		let addr = (self.allocate)(self as *mut _);
		return Some(Frame::with_address(PhysicalAddress(addr as u64)));
	}

	fn deallocate_frame(&mut self, addr: Frame) {
		(self.deallocate)(self as *mut _, addr.start_address().0 as *mut c_void);
	}
}
