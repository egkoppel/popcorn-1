/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
