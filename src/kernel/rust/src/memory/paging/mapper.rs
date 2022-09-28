/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use crate::memory::{Page, Frame, PhysicalAddress, VirtualAddress};
use crate::memory::frame_alloc::Allocator;

use super::tables::{PageTable, Level4};
use core::arch::asm;
use super::tables::EntryFlags;

pub struct Mapper<'a> {
	p4: &'a mut PageTable<Level4>,
}

impl Mapper<'_> {
	pub fn new<'a>(p4: &'a mut PageTable<Level4>) -> Mapper<'a> {
		return Mapper { p4: p4 };
	}

	pub fn map_page_to(&mut self, page: Page, frame: Frame, flags: EntryFlags, allocator: &mut dyn Allocator) {
		let p3 = self.p4.get_child_table_or_new(page.start_address().p4_index(), allocator);
		let p2 = p3.get_child_table_or_new(page.start_address().p3_index(), allocator);
		let p1 = p2.get_child_table_or_new(page.start_address().p2_index(), allocator);
		p1[page.start_address().p1_index()].set_address(frame);
		p1[page.start_address().p1_index()].set_flags(flags);
		p2[page.start_address().p2_index()].add_flags(flags);
		p3[page.start_address().p3_index()].add_flags(flags);
		self.p4[page.start_address().p4_index()].add_flags(flags);
	}

	pub fn map_page(&mut self, page: Page, flags: EntryFlags, allocator: &mut dyn Allocator) {
		let frame_ = allocator.allocate_frame();
		assert!(frame_.is_some(), "Couldn't allocate frame to map to {:?}", page);
		let frame = frame_.unwrap();
		self.map_page_to(page, frame, flags, allocator);
	}

	pub fn unmap_page(&mut self, page: Page, allocator: &mut dyn Allocator) {
		let p1 = self.p4
		             .get_child_table_mut(page.start_address().p4_index())
		             .and_then(|p3| p3.get_child_table_mut(page.start_address().p3_index()))
		             .and_then(|p2| p2.get_child_table_mut(page.start_address().p2_index()))
		             .expect("Attempted to unmap unmapped address");
		let entry = &mut p1[page.start_address().p1_index()];
		allocator.deallocate_frame(entry.get_address().unwrap());
		entry.clear();
		unsafe { asm!("invlpg [{}]", in(reg) page.start_address().0); }
	}

	pub fn unmap_page_no_free(&mut self, page: Page) {
		let p1 = self.p4
		             .get_child_table_mut(page.start_address().p4_index())
		             .and_then(|p3| p3.get_child_table_mut(page.start_address().p3_index()))
		             .and_then(|p2| p2.get_child_table_mut(page.start_address().p2_index()))
		             .expect("Attempted to unmap unmapped address");
		let entry = &mut p1[page.start_address().p1_index()];
		entry.clear();
		unsafe { asm!("invlpg [{}]", in(reg) page.start_address().0); }
	}

	pub fn translate_page(&self, page: Page) -> Option<Frame> {
		return self.p4.get_child_table(page.start_address().p4_index())
		           .and_then(|p3| p3.get_child_table(page.start_address().p3_index()))
		           .and_then(|p2| p2.get_child_table(page.start_address().p2_index()))
		           .and_then(|p1| p1[page.start_address().p1_index()].get_address());
	}

	pub fn translate_address(&self, addr: VirtualAddress) -> Option<PhysicalAddress> {
		return self.translate_page(Page::containing_address(addr))
		           .and_then(|frame| Some(frame.start_address() + addr.p1_offset()));
	}
}