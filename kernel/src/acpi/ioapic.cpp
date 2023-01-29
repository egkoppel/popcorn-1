/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ioapic.hpp"

namespace acpi {
	ioapic_t::ioapic_t(memory::paddr_t ioapic_base, u32 gsi_start, memory::IPhysicalAllocator& allocator)
		: registers(ioapic_base, 0x14, flags, allocator, memory::paging::kas),
		  gsi_start(gsi_start) {
		auto irq_count = (((*this)->version() >> 16) & 0xFFF) + 1;
		LOG(Log::INFO, "IOAPIC handles %u irqs", irq_count);
		this->gsi_end = this->gsi_start + irq_count;
	}

	void Ioapics::add_ioapic(ioapic_t&& ioapic) {
		this->ioapics.push_back(std::move(ioapic));
	}

	void Ioapics::add_iso(u32 irq, u32 gsi, u32 flags) {
		this->isos.emplace_back(irq, gsi);
		auto ioapic                         = this->find_ioapic_for_gsi(gsi).value();
		bool active_low                     = flags & 2;
		bool level_triggered                = flags & 8;
		auto redirection_entry              = (*ioapic)->redirections()[gsi - ioapic->gsi_start];
		redirection_entry.level_triggered() = level_triggered;
		redirection_entry.active_low()      = active_low;
	}

	u32 Ioapics::pic_irq_to_gsi(u32 irq) {
		for (auto&& iso : this->isos) {
			if (iso.first == irq) return iso.second;
		}
		return irq;
	}

	std::optional<ioapic_t *> Ioapics::find_ioapic_for_gsi(u32 gsi) {
		for (auto& ioapic : this->ioapics) {
			if (ioapic.gsi_start <= gsi && gsi < ioapic.gsi_end) return &ioapic;
		}
		return std::nullopt;
	}

	redirection_entry Ioapics::redirection_entry(u32 gsi) {
		auto& ioapic = *this->find_ioapic_for_gsi(gsi).value();
		return ioapic->redirections()[gsi - ioapic.gsi_start];
	}

	namespace detail {
		ioapic_register::ioapic_register(u32 num, ioapic_registers *ioapic) : ioapic(ioapic), num(num) {}

		ioapic_register::operator u32() volatile {
			*this->ioapic->reg_sel() = this->num;
			return *this->ioapic->reg_win();
		}

		volatile ioapic_register& ioapic_register::operator=(u32 val) volatile {
			*this->ioapic->reg_sel() = this->num;
			*this->ioapic->reg_win() = val;
			return *this;
		}
		volatile ioapic_register& ioapic_register::operator&=(u32 val) volatile {
			*this->ioapic->reg_sel() = this->num;
			*this->ioapic->reg_win() &= val;
			return *this;
		}
		volatile ioapic_register& ioapic_register::operator|=(u32 val) volatile {
			*this->ioapic->reg_sel() = this->num;
			*this->ioapic->reg_win() |= val;
			return *this;
		}


		ioapic_register_64::ioapic_register_64(u32 num, ioapic_registers *ioapic) : ioapic(ioapic), num_lower(num) {}

		ioapic_register_64::operator u64() volatile {
			*this->ioapic->reg_sel() = this->num_lower;
			auto low                 = *this->ioapic->reg_win();
			*this->ioapic->reg_sel() = this->num_lower + 1;
			auto high                = *this->ioapic->reg_win();
			return (static_cast<u64>(high) << 32) | low;
		}

		volatile ioapic_register_64& ioapic_register_64::operator=(u64 val) volatile {
			u32 high = static_cast<u32>(val >> 32);
			u32 low  = val & 0xFFFFFFFF;

			*this->ioapic->reg_sel() = this->num_lower;
			*this->ioapic->reg_win() = low;
			*this->ioapic->reg_sel() = this->num_lower + 1;
			*this->ioapic->reg_win() = high;
			return *this;
		}
		volatile ioapic_register_64& ioapic_register_64::operator&=(u64 val) volatile {
			u32 high = static_cast<u32>(val >> 32);
			u32 low  = val & 0xFFFFFFFF;

			*this->ioapic->reg_sel() = this->num_lower;
			*this->ioapic->reg_win() &= low;
			*this->ioapic->reg_sel() = this->num_lower + 1;
			*this->ioapic->reg_win() &= high;
			return *this;
		}
		volatile ioapic_register_64& ioapic_register_64::operator|=(u64 val) volatile {
			u32 high = static_cast<u32>(val >> 32);
			u32 low  = val & 0xFFFFFFFF;

			*this->ioapic->reg_sel() = this->num_lower;
			*this->ioapic->reg_win() |= low;
			*this->ioapic->reg_sel() = this->num_lower + 1;
			*this->ioapic->reg_win() |= high;
			return *this;
		}
		volatile ioapic_register_64& ioapic_register_64::operator^=(u64 val) volatile {
			u32 high = static_cast<u32>(val >> 32);
			u32 low  = val & 0xFFFFFFFF;

			*this->ioapic->reg_sel() = this->num_lower;
			*this->ioapic->reg_win() ^= low;
			*this->ioapic->reg_sel() = this->num_lower + 1;
			*this->ioapic->reg_win() ^= high;
			return *this;
		}

		ioapic_redirection_registers::ioapic_redirection_registers(ioapic_registers *ioapic) : ioapic(ioapic) {}

		struct redirection_entry ioapic_redirection_registers::operator[](usize irq) {
			return {
					.reg = {static_cast<u32>(0x10 + irq * 2), this->ioapic}
            };
		}
	}   // namespace detail
}   // namespace acpi